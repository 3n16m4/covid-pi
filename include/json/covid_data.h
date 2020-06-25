#ifndef COVID_PI_COVID_DATA_H
#define COVID_PI_COVID_DATA_H

#include <array>
#include <cstdint>

/**
 * Example JSON Data:
 *
 * {
      "location": "Germany",
      "country_code": "de",
      "latitude": 51.165691,
      "longitude": 10.451526,
      "confirmed": 177289,
      "dead": 8123,
      "recovered": 154600,
      "updated": "2020-05-19 07:15:06.108706+00:00"
    }

    {
      "location": "Niedersachsen",
      "country_code": "de",
      "latitude": 52.6367036,
      "longitude": 9.8450766,
      "confirmed": 11207,
      "dead": 548,
      "recovered": null,
      "updated": "2020-05-19 07:47:53.511813+00:00"
    }

    {
      "location": "Tokyo",
      "country_code": "jp",
      "latitude": 35.6761919,
      "longitude": 139.6503106,
      "confirmed": 5073,
      "dead": 241,
      "recovered": 3632,
      "updated": "2020-05-19 07:48:43.413081+00:00"
    }

    latitude, longitude, updated fields are ignored.
 */

static constexpr auto MAX_COUNTRY_NAME_LEN = 32;
static constexpr auto MAX_COUNTRY_CODE_LEN = 2;

struct covid_data final {
    std::array<char, MAX_COUNTRY_NAME_LEN + 1> name{};
    // ISO 3166-1 alpha-2
    std::array<char, MAX_COUNTRY_CODE_LEN + 1> code{};
    std::int32_t confirmed{};
    std::int32_t dead{};
    std::int32_t recovered{};

    constexpr bool operator<(covid_data const &rhs) const noexcept {
        return confirmed < rhs.confirmed;
    }

    constexpr bool operator>(covid_data const &rhs) const noexcept {
        return confirmed > rhs.confirmed;
    }
};

#endif // COVID_PI_COVID_DATA_H
