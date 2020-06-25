#ifndef COVID_PI_OLED_DISPLAY_H
#define COVID_PI_OLED_DISPLAY_H

#include <fmt/format.h>
#include <fmt/core.h>

extern "C" {
#include "ssd1306_i2c/ssd1306_i2c.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
}

#include <string_view>
#include <cstdint>

struct oled_display final {
    /// Sets the hardware up.
    /// \param vcc_state default should be SSD1306_SWITCHCAPVCC
    /// \param i2c_addr default should be SSD1306_I2C_ADDRESS
    /// \return true if hardware got initialized successfully, otherwise false.
    [[nodiscard]] static bool setup(std::uintptr_t vcc_state, std::int32_t i2c_addr) noexcept;

    /// Turns the display off
    static void cleanup() noexcept;

    /// Renders the internal display buffer.
    static void render() noexcept;

    /// Clears the internal display buffer.
    static void clear_buffer() noexcept;

    /// Clears the display and renders it.
    static void clear() noexcept;

    /// Send a command via I2C to the Display.
    /// \param command the command
    static void send_command(std::uint8_t command) noexcept;

    static void set_text_size(std::uint8_t size) noexcept;

    /// Convenient way to format data and write to the internal display buffer.
    /// This function does *not* allocate any dynamic memory.
    /// \tparam Args Variadic template argument
    /// \param fmt The format string ({}, ...)
    /// \param args The arguments to be formatted
    template <typename... Args>
    static void write(std::uint8_t x, std::uint8_t y, std::string_view fmt, Args const &... args) {
        auto constexpr buffer_size = SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8;
        std::array<char, buffer_size> buf{};
        fmt::format_to_n(buf.data(), buf.size(), fmt, args...);
        ssd1306_drawText(x, y, buf.data());
    }

    /// Writes and renders the internal display buffer.
    /// \tparam Args Variadic template argument
    /// \param fmt The format string ({}, ...)
    /// \param args The arguments to be formatted
    template <typename... Args>
    static void display(std::uint8_t x, std::uint8_t y, std::string_view fmt, Args const &... args) {
        write(x, y, fmt, args...);
        render();
    }
};

#endif // COVID_PI_OLED_DISPLAY_H
