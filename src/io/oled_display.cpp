#include <include/io/oled_display.h>
#include <include/utils.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

bool oled_display::setup(std::uintptr_t vcc_state, std::uintptr_t i2c_addr) noexcept {
    if (wiringPiSetup() != 0) {
        std::fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
        return false;
    }
    cleanup();
    return ssd1306_begin(vcc_state, i2c_addr) == 0;
}

void oled_display::cleanup() noexcept {
    // Turn display off
    ssd1306_command(SSD1306_DISPLAYOFF);

    // Switch off LEDs
    digitalWrite(io::gpio_pins::LED_GREEN, LOW);
    digitalWrite(io::gpio_pins::LED_RED, LOW);
}

void oled_display::render() noexcept {
    ssd1306_display();
}

void oled_display::clear_buffer() noexcept {
    ssd1306_clearDisplay();
}

void oled_display::clear() noexcept {
    clear_buffer();
    render();
}

void oled_display::send_command(std::uint8_t command) noexcept {
    ssd1306_command(command);
}

void oled_display::set_text_size(std::uint8_t size) noexcept {
    ssd1306_setTextSize(size);
}
