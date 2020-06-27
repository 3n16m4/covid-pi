#include <include/io/menu.h>
#include <include/io/oled_display.h>
#include <include/json/covid_data.h>
#include <include/utils.h>

#include <fmt/core.h>

namespace io {
    void menu::add_menu(menu::pages_type &&pages) noexcept {
        pages_ = std::move(pages);
        render();
    }

    void menu::render() noexcept {
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
                         loc.data(), code.data(), confirmed, dead, recovered);
        std::scoped_lock<std::mutex> lk(display_mutex_);
        oled_display::clear_buffer();
        oled_display::write(0, MenuRow::ROW7, "Page: {}/{}", index_ + 1,
                            size());
        oled_display::display(0, 0, buffer.data());
    }

    menu::pages_type const &menu::pages() const noexcept {
        return pages_;
    }

    menu::pages_type &menu::pages() noexcept {
        return pages_;
    }

    void menu::prev() noexcept {
        index_ == 0 ? (index_ = size() - 1) : (index_--);
    }

    void menu::next() noexcept {
        index_ = (index_ + 1) % size();
    }

    menu::size_type menu::size() const noexcept {
        return pages_.size();
    }

    covid_data const *menu::current() const noexcept {
        return pages_[index_].get();
    }

    covid_data *menu::current() noexcept {
        return pages_[index_].get();
    }
} // namespace io