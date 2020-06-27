#ifndef COVID_PI_MENU_H
#define COVID_PI_MENU_H

#include "../json/covid_data.h"

#include <cstdint>
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

        /**
         *  @brief   Adds the menu pages using move sementics.
         */
        void add_menu(pages_type &&pages) noexcept;

        /**
         *  @brief Renders the current page to the OLED display.
         */
        void render() noexcept;

        /**
         *  @brief  Returns an immutable reference of the menu pages.
         */
        [[nodiscard]] pages_type const &pages() const noexcept;

        /**
         *  @brief  Returns a mutable reference of the menu pages.
         */
        [[nodiscard]] pages_type &pages() noexcept;

        /**
         *  @brief  Sets the current page to the previous one.
         */
        void prev() noexcept;

        /**
         *  @brief  Sets the current page to the next one.
         */
        void next() noexcept;

        /**
         *  @brief  Returns the total number of pages.
         */
        [[nodiscard]] size_type size() const noexcept;

        /**
         *  @brief  Returns the current immutable page.
         */
        [[nodiscard]] covid_data const *current() const noexcept;

        /**
         *  @brief  Returns the current mutable page.
         */
        [[nodiscard]] covid_data *current() noexcept;

      private:
        std::mutex display_mutex_;
        size_type index_;
        pages_type pages_;
    };
} // namespace io

#endif // COVID_PI_MENU_H
