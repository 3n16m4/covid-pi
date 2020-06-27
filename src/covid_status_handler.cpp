#include <include/covid_status_handler.h>
#include <include/io/oled_display.h>
#include <include/utils.h>

#include <future>
#include <mutex>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <curl/curl.h>

/**
 * @brief   The callback for writing received data.
 * @return  Returns the number of bytes written to data.
 */
static size_t write_callback(char *ptr, size_t size, size_t nmemb,
                             void *userdata) {
    reinterpret_cast<std::string *>(userdata)->append(ptr, size * nmemb);
    return size * nmemb;
}

covid_status_handler::covid_status_handler(io::menu &menu,
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

covid_status_handler::~covid_status_handler() {
    curl_easy_cleanup(handle_);
    curl_global_cleanup();
}

void covid_status_handler::set_mode(APIType api_type) noexcept {
    assert(api_type <= 1 && api_type >= 0 && "APIType out of range!");
    curl_easy_setopt(handle_, CURLOPT_URL, apis[api_type]);
}

void covid_status_handler::set_timeout(long timeout) noexcept {
    curl_easy_setopt(handle_, CURLOPT_TIMEOUT, timeout);
}

bool covid_status_handler::setup() noexcept {
    if (handle_ == nullptr) {
        return false;
    }
    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &json_data_);
    curl_easy_setopt(handle_, CURLOPT_USERAGENT, "covid-pi/1.0");
    return true;
}

bool covid_status_handler::request() noexcept {
    auto const res = curl_easy_perform(handle_);
    if (res != CURLE_OK) {
        fmt::print(stderr, "curl_easy_perform() failed: {}\n",
                   curl_easy_strerror(res));
        return false;
    }
    return handle_data_received();
}

bool covid_status_handler::async_request() {
    using namespace std::literals::chrono_literals;

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

    return task.get() && handle_data_received();
}

bool covid_status_handler::handle_data_received() {
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
        using namespace utils;

        auto page = std::make_unique<covid_data>();
        auto const &code = e["country_code"].Move().GetString();
        // filter by country if given
        if (!country_.empty() && code != country_) {
            continue;
        }

        auto &&loc = std::string{e["location"].Move().GetString()};
        replace_umlauts(loc);

        auto const &confirmed =
            json_default_val<std::int32_t>(e["confirmed"], 0);
        auto const &dead = json_default_val<std::int32_t>(e["dead"], 0);
        auto const &recovered =
            json_default_val<std::int32_t>(e["recovered"], 0);
        std::copy_n(std::begin(loc), page->name.max_size() - 1,
                    std::begin(page->name));
        std::copy_n(code, page->code.max_size() - 1, std::begin(page->code));
        page->confirmed = confirmed;
        page->dead = dead;
        page->recovered = recovered;

        pages.emplace_back(std::move(page));
    }
    if (pages.empty()) {
        fmt::print(stderr, "Country with code {} has no registered cities!\n",
                   country_);
        return false;
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