#include <include/io/input_handler.h>
#include <include/io/menu.h>
#include <include/io/oled_display.h>
#include <include/utils.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <functional>
#include <future>
#include <mutex>
#include <string_view>

#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <cxxopts.hpp>

using namespace std::literals::chrono_literals;

// clang-format off
enum APIType : std::uint8_t { 
    Countries,
    Cities
};
// clang-format on

class covid_status_handler final {
  public:
    using SortFunction = std::function<bool(io::menu::page_type const &,
                                            io::menu::page_type const &)>;

    static constexpr std::array<char const *, 2> apis{
        "https://www.trackcorona.live/api/countries",
        "https://www.trackcorona.live/api/cities"};

    explicit covid_status_handler(io::menu &menu,
                                  io::input_handler &input_handler,
                                  std::string_view country,
                                  SortFunction &&sort_fun)
        : menu_(menu), input_handler_(input_handler), country_(country),
          sort_fun_(std::move(sort_fun)) {
        curl_global_init(CURL_GLOBAL_ALL);
        handle_ = curl_easy_init();
        // pre-allocate 40KB
        json_data_.reserve(4096 * 10);
    }

    ~covid_status_handler() {
        curl_easy_cleanup(handle_);
        curl_global_cleanup();
    }

    /**
     *  @brief  Specifies the API URL.
     *  @param  api_type    The API Type.
     */
    void set_mode(APIType api_type) noexcept {
        assert(api_type <= 1 && api_type >= 0 && "APIType out of range!");
        curl_easy_setopt(handle_, CURLOPT_URL, apis[api_type]);
    }

    /**
     *  @brief  Sets the maximum timeout to wait for a request to finish.
     *  @param timeout  The maximum timeout in seconds before the request should
     * be aborted.
     */
    void set_timeout(long timeout) noexcept {
        curl_easy_setopt(handle_, CURLOPT_TIMEOUT, timeout);
    }

    /**
     *  @brief  Prepare the Curl Request.
     *  @return  True if successful, otherwise false.
     */
    [[nodiscard]] bool setup() noexcept {
        if (handle_ == nullptr) {
            return false;
        }
        curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &json_data_);
        curl_easy_setopt(handle_, CURLOPT_USERAGENT, "covid-pi/1.0");
        return true;
    }

    /**
     *  @brief  Performs a blocking GET request.
     *  @return True if successful, otherwise false.
     */
    [[nodiscard]] bool request() noexcept {
        auto const res = curl_easy_perform(handle_);
        if (res != CURLE_OK) {
            fmt::print(stderr, "curl_easy_perform() failed: {}\n",
                       curl_easy_strerror(res));
            return false;
        }
        return handle_data_received();
    }

    /**
     *  @brief  Performs an async non-blocking GET request.
     *  @return True if successful, otherwise false.
     */
    [[nodiscard]] bool async_request() {
        json_data_.clear();
        auto task = std::async(std::launch::async, [this]() -> bool {
            assert(handle_ && "curl handle is NULL!\n");
            auto const res = curl_easy_perform(handle_);
            if (res != CURLE_OK) {
                fmt::print(stderr, "curl_easy_perform() failed: {}\n",
                           curl_easy_strerror(res));
                return false;
            }
            return true;
        });
        // poll the status and perform parallel work
        std::future_status status;
        bool toggle{false};
        do {
            // toggle the LEDs every 250ms
            status = task.wait_for(250ms);
            if (status == std::future_status::timeout) {
                // do some parallel work here
                // toggle the LEDs
                fmt::print("waiting... {}\n", toggle);
                toggle = !toggle;
                if (toggle) {
                    digitalWrite(io::gpio_pins::LED_GREEN, HIGH);
                    digitalWrite(io::gpio_pins::LED_RED, LOW);
                } else {
                    digitalWrite(io::gpio_pins::LED_GREEN, LOW);
                    digitalWrite(io::gpio_pins::LED_RED, HIGH);
                }
            }
        } while (status != std::future_status::ready);

        fmt::print("Task finished!\n");
        return task.get() && handle_data_received();
    }

    /**
     * @brief   The callback for writing received data.
     * @return  Returns the number of bytes written to data.
     */
    static size_t write_callback(char *ptr, size_t size, size_t nmemb,
                                 void *userdata) {
        reinterpret_cast<std::string *>(userdata)->append(ptr, size * nmemb);
        return size * nmemb;
    }

  private:
    /**
     *  @brief  The callback when the data was completely received.
     *  @return True if successful, otherwise false.
     */
    bool handle_data_received() {
        using namespace rapidjson;

        Document d;
        ParseResult const ok = d.Parse(json_data_.c_str());
        if (!ok) {
            fmt::print(stderr, "JSON parse error: {} ({})",
                       GetParseError_En(ok.Code()), ok.Offset());
            return false;
        }

        io::menu::pages_type pages{};
        pages.reserve(d["data"].GetArray().Size());

        // move data from json document into covid_status vector
        for (auto &&e : d["data"].GetArray()) {
            auto &&page = pages.emplace_back(std::make_unique<covid_data>());

            auto &&loc = e["location"].Move().GetString();
            auto &&code = e["country_code"].Move().GetString();

            // TODO: filter country here

            using namespace utils;
            auto const &confirmed =
                json_default_val<std::int32_t>(e["confirmed"], 0);
            auto const &dead = json_default_val<std::int32_t>(e["dead"], 0);
            auto const &recovered =
                json_default_val<std::int32_t>(e["recovered"], 0);
            std::copy_n(std::move(loc), page->name.max_size() - 1,
                        std::begin(page->name));
            std::copy_n(std::move(code), page->code.max_size() - 1,
                        std::begin(page->code));
            page->confirmed = confirmed;
            page->dead = dead;
            page->recovered = recovered;
        }
        std::sort(std::begin(pages), std::end(pages), sort_fun_);
        {
            // put input handler thread to sleep until data from mainthread is
            // ready.
            std::lock_guard<std::mutex> lk(input_handler_.mutex());
            input_handler_.ready(false);
            menu_.add_menu(std::move(pages));
            input_handler_.ready(true);
        }
        input_handler_.cv().notify_one();

        // turn off the status LEDs
        digitalWrite(io::gpio_pins::LED_GREEN, LOW);
        digitalWrite(io::gpio_pins::LED_RED, LOW);
        return true;
    }

  private:
    CURL *handle_;
    std::string json_data_;

    io::menu &menu_;
    io::input_handler &input_handler_;

    std::string_view country_;
    SortFunction sort_fun_;
};

int main(int argc, char *argv[]) {
    static_assert(
        utils::alpha_2_codes.max_size() == 676,
        "Alpha-2-Codes list size does not match official specification!");

    // default arguments
    APIType api_mode{APIType::Countries};
    std::string_view country{"all"};
    covid_status_handler::SortFunction sort_fun =
        [](auto &&lhs, auto &&rhs) noexcept -> bool {
        return lhs->confirmed > rhs->confirmed;
    };

    // parse optional command line arguments
    try {
        cxxopts::Options options(argv[0], "a covid-19 live tracker.");

        // clang-format off
        options.add_options()
            ("h, help", "Print usage")
            ("c, country", "Filter by country and show its cities", cxxopts::value<std::string>(), "alpha-2 code")
            ("s, sort", "Sort by confirmed cases.", cxxopts::value<std::string>(), "low / high")
        ;
        // clang-format on
        auto const result = options.parse(argc, argv);
        if (result.count("help")) {
            fmt::print(options.help());
            return EXIT_SUCCESS;
        }
        if (result.count("country")) {
            api_mode = APIType::Cities;
            auto const &res = result["country"].as<std::string>();
            if (res.size() != 2) {
                fmt::print(
                    stderr,
                    "Invalid alpha-2 country code! See "
                    "https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2\n");
                return EXIT_SUCCESS;
            } else {
                country = res;
            }
        }
        if (result.count("sort")) {
            auto const order = result["sort"].as<std::string>();
            if (order == "low") {
                sort_fun = [](auto &&lhs, auto &&rhs) noexcept -> bool {
                    return lhs->confirmed < rhs->confirmed;
                };
            } else if (order == "high") {
                sort_fun = [](auto &&lhs, auto &&rhs) noexcept -> bool {
                    return lhs->confirmed > rhs->confirmed;
                };
            } else {
                fmt::print(
                    stderr,
                    "Invalid sort order. Available options: low, high\n");
                return EXIT_SUCCESS;
            }
        }
    } catch (cxxopts::OptionException const &e) {
        fmt::print(stderr, "Error parsing options: {}\n", e.what());
        return EXIT_FAILURE;
    }

    // Setup wiringPi and the i2c interface
    if (!oled_display::setup(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
        return EXIT_FAILURE;
    }
    // Install signal handler
    if (std::signal(SIGINT, [](int signal) {
            oled_display::cleanup();
            std::exit(signal);
        }) == SIG_ERR) {
        return EXIT_FAILURE;
    }

    constexpr std::string_view loading_text{"Loading..."};
    // clear display splashscreen
    oled_display::clear_buffer();
    oled_display::set_text_size(2);
    oled_display::display(0, io::MenuRow::ROW3, loading_text);
    oled_display::set_text_size(1);

    // initialize io
    pinMode(io::gpio_pins::BTN_LEFT, INPUT);
    pinMode(io::gpio_pins::BTN_RIGHT, INPUT);
    pinMode(io::gpio_pins::LED_GREEN, OUTPUT);
    pinMode(io::gpio_pins::LED_RED, OUTPUT);

    // initialize menu
    io::menu menu{};
    io::input_handler input_handler{menu};
    input_handler.start();

    covid_status_handler status_handler{menu, input_handler, country,
                                        std::move(sort_fun)};
    status_handler.set_mode(api_mode);
    if (!status_handler.setup()) {
        fmt::print(stderr, "curl setup failed!\n");
        return EXIT_FAILURE;
    }
    // 60 seconds request timeout
    status_handler.set_timeout(60L);

    constexpr auto next_request_time = 20min;
    bool done{false};
    while (!done) {
        // perform the async request
        if (!status_handler.async_request()) {
            done = false;
            break;
        }
        // perform next request after 20 minutes
        std::this_thread::sleep_for(next_request_time);
    }

    // wait for the io thread to finish
    input_handler.request_interrupt();
    input_handler.wait();

    oled_display::cleanup();

    return EXIT_SUCCESS;
}
