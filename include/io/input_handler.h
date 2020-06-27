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
        /**
         *  @brief  Constructor.
         *  @param  menu  A menu reference.
         */
        explicit input_handler(io::menu &menu);

        /**
         *  @brief  Starts an asynchronous thread for handling input.
         */
        void start();

        /**
         *  @brief  Notifies the input thread to be stopped.
         */
        void request_interrupt() noexcept;

        /**
         *  @brief  Waits for the input thread to finish.
         */
        void wait() const noexcept;

        /**
         *  @brief  Sets the ready flag for the input thread to determine when
         *          to wait or wake up itself.
         *  @param  ready true or false.
         */
        void ready(bool ready) noexcept;

        /**
         *  @brief  The condition_variable for the input thread.
         *  @return condition_variable for input thread.
         */
        std::condition_variable &cv();

        /**
         *  @brief  The mutex for the input thread.
         *  @return   mutex for the input thread.
         */
        std::mutex &mutex();

      private:
        /**
         *  @brief  The input thread itself. Handles the input and the LCD Menu.
         */
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
