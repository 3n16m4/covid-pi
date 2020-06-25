#ifndef COVID_PI_UTILS_H
#define COVID_PI_UTILS_H

#include <rapidjson/document.h>

#include <cstdint>
#include <limits>

constexpr auto modulo(std::int32_t value, std::uint32_t m) noexcept {
    auto const mod = value % static_cast<std::int32_t>(m);
    m &= value >> std::numeric_limits<int>::digits;
    return mod + m;
}

template <typename T>
auto const json_default_val =
    [](rapidjson::Value const &val, T const &default_val) { return val.Is<T>() ? val.Get<T>() : default_val; };

namespace io {
    // clang-format off
    enum gpio_pins : std::uint8_t {
        LED_GREEN = 0,
        LED_RED = 2,
        BTN_LEFT = 21,
        BTN_RIGHT = 22,
    };
    // clang-format on
} // namespace io

#endif // COVID_PI_UTILS_H
