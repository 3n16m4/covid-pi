#include <include/io/input_handler.h>
#include <include/io/oled_display.h>
#include <include/io/menu.h>

io::input_handler::input_handler(io::menu &m) : menu_(m) {
}

void io::input_handler::start() {
    io_thread_ = std::async(std::launch::async, [this]() { process_inputs_thread(); });
}

void io::input_handler::request_interrupt() {
    stop_token_.store(false);
}

void io::input_handler::wait() const {
    io_thread_.wait();
}

void io::input_handler::process_inputs_thread() {
    for (;;) {
        // wait until main thread has data
        std::unique_lock<std::mutex> ul(menu_mutex_);
        cv_.wait(ul, [&] { return ready_; });
        ul.unlock();

        if (stop_token_.load()) {
            return;
        }
        if (digitalRead(io::gpio_pins::BTN_LEFT) == HIGH) {
            menu_.prev();
            menu_.render();
        }
        else if (digitalRead(io::gpio_pins::BTN_RIGHT) == HIGH) {
            menu_.next();
            menu_.render();
        }
        std::this_thread::sleep_for(DEBOUNCE_TIME);
    }
}

std::condition_variable &io::input_handler::cv() {
    return cv_;
}

std::mutex &io::input_handler::mutex() {
    return menu_mutex_;
}

void io::input_handler::ready(bool ready) noexcept {
    ready_ = ready;
}
