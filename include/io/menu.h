#ifndef COVID_PI_MENU_H
#define COVID_PI_MENU_H

#include "../json/covid_data.h"
#include "../utils.h"
#include "oled_display.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <rapidjson/document.h>

namespace io {
    enum MenuRow : std::uint8_t {
        ROW1 = 1 * 8,
        ROW2 = 2 * 8,
        ROW3 = 3 * 8,
        ROW4 = 4 * 8,
        ROW5 = 5 * 8,
        ROW6 = 6 * 8,
        ROW7 = 7 * 8,
    };

    class menu final {
      public:
        using page_type = std::unique_ptr<covid_data>;
        using pages_type = std::vector<page_type>;
        using size_type = pages_type::size_type;

        void add_menu(pages_type &&pages) noexcept {
            pages_ = std::move(pages);
            render();
        }

        /// Renders the current page to the display.
        /// Thread-Safe: Yes
        void render() noexcept {
            auto *const page = current();
            auto const &loc = page->name;
            auto const &code = page->code;
            auto const &confirmed = page->confirmed;
            auto const &dead = page->dead;
            auto const &recovered = page->recovered;

            // 192 bytes should be enough
            std::array<char, 192> buffer{};
            fmt::format_to_n(buffer.data(), buffer.size(),
                             "Location: {}\n"
                             "Code: {}\n"
                             "Cases: {}\n"
                             "Dead: {}\n"
                             "Healed: {}",
                             loc.data(), code.data(), confirmed, dead,
                             recovered);
            std::scoped_lock<std::mutex> lk(display_mutex_);
            oled_display::clear_buffer();
            oled_display::write(0, MenuRow::ROW7, "*** Page {}/{} ***",
                                index_ + 1, size());
            oled_display::display(0, 0, buffer.data());
        }

        [[nodiscard]] auto const &pages() const noexcept {
            return pages_;
        }

        [[nodiscard]] auto &pages() noexcept {
            return pages_;
        }

        /// Sets the current page to the previous one.
        /// Thread-Safe: No
        void prev() noexcept {
            index_ == 0 ? (index_ = size() - 1) : (index_--);
        }

        /// Sets the current page to the next one.
        /// Thread-Safe: No
        void next() noexcept {
            index_ = (index_ + 1) % size();
        }

        /// Returns the total number of pages.
        /// Thread-Safe: No
        [[nodiscard]] size_type size() const noexcept {
            return pages_.size();
        }

        /// Returns the current immutable page.
        /// Thread-Safe: No
        [[nodiscard]] covid_data const *current() const noexcept {
            return pages_[index_].get();
        }

        /// Returns the current mutable page.
        /// Thread-Safe: No
        [[nodiscard]] covid_data *current() noexcept {
            return pages_[index_].get();
        }

      private:
        std::mutex display_mutex_;
        size_type index_;
        pages_type pages_;
    };
} // namespace io

#endif // COVID_PI_MENU_H
