#ifndef COVID_PI_INPUT_HANDLER_H
#define COVID_PI_INPUT_HANDLER_H

#include "../include/utils.h"

#include <wiringPi.h>

#include <chrono>
#include <mutex>
#include <atomic>
#include <future>

namespace io {
    class menu;

    using namespace std::literals::chrono_literals;

    class input_handler final {
        static constexpr auto DEBOUNCE_TIME = 50ms;

      public:
        explicit input_handler(io::menu &m);

        void start();

        void request_interrupt();

        void wait() const;

        void ready(bool ready) noexcept;

        std::condition_variable &cv();

        std::mutex &mutex();

      private:
        void process_inputs_thread();

      private:
        std::mutex menu_mutex_;
        std::atomic<bool> stop_token_{false};
        std::future<void> io_thread_;
        io::menu &menu_;
        std::condition_variable cv_{};
        bool ready_{false};
    };
} // namespace io

#endif // COVID_PI_INPUT_HANDLER_H
