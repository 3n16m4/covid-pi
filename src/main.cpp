#include <include/covid_status_handler.h>
#include <include/io/oled_display.h>
#include <include/utils.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <cxxopts.hpp>
#include <fmt/core.h>

using namespace std::literals::chrono_literals;

int main(int argc, char *argv[]) {
    using namespace utils;

    // default arguments
    APIType api_mode{APIType::Countries};
    std::string country;
    covid_status_handler::SortFunction sort_fun =
        [](auto &&lhs, auto &&rhs) noexcept -> bool {
        return lhs->confirmed > rhs->confirmed;
    };

    // parse optional command line arguments
    try {
        cxxopts::Options options(argv[0], "A covid-19 live tracker.");

        // clang-format off
        options.add_options()
            ("h, help", "Print usage")
            ("c, cities", "Filter by country and show its cities", cxxopts::value<std::string>(), "alpha-2 code")
            ("s, sort", "Sort by confirmed cases.", cxxopts::value<std::string>(), "low / high")
        ;
        // clang-format on
        auto const result = options.parse(argc, argv);
        if (result.count("help")) {
            fmt::print(options.help());
            return EXIT_SUCCESS;
        }
        if (result.count("cities")) {
            api_mode = APIType::Cities;
            auto const res = result["cities"].as<std::string>();
            if (!utils::has_alpha_2_code(res)) {
                fmt::print(
                    stderr,
                    "Invalid alpha-2 country code! See "
                    "https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2\n");
                return EXIT_SUCCESS;
            }
            country = res;
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
    if (!io::oled_display::setup(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
        return EXIT_FAILURE;
    }
    // Install signal handler
    if (std::signal(SIGINT, [](int signal) {
            io::oled_display::cleanup();
            std::exit(signal);
        }) == SIG_ERR) {
        return EXIT_FAILURE;
    }

    constexpr std::string_view loading_text{"Loading..."};
    // clear display splashscreen
    io::oled_display::clear_buffer();
    io::oled_display::set_text_size(2);
    io::oled_display::display(0, io::MenuRow::ROW3, loading_text);
    io::oled_display::set_text_size(1);

    // initialize io
    pinMode(io::gpio_pins::BTN_LEFT, INPUT);
    pinMode(io::gpio_pins::BTN_RIGHT, INPUT);
    pinMode(io::gpio_pins::LED_GREEN, OUTPUT);
    pinMode(io::gpio_pins::LED_RED, OUTPUT);

    // initialize menu
    io::menu menu{};
    io::input_handler input_handler{menu};
    input_handler.start();

    covid_status_handler status_handler{menu, input_handler, std::move(country),
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
            fmt::print(stderr, "async_request() failed!\n");
            done = true;
            break;
        }
        // perform next request after 20 minutes
        std::this_thread::sleep_for(next_request_time);
    }

    // wait for the io thread to finish
    {
        std::lock_guard<std::mutex> lk(input_handler.mutex());
        input_handler.ready(true);
    }
    input_handler.cv().notify_one();
    input_handler.request_interrupt();
    input_handler.wait();

    io::oled_display::cleanup();

    return EXIT_SUCCESS;
}
