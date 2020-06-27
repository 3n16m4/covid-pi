#ifndef COVID_PI_COVID_STATUS_HANDLER_H
#define COVID_PI_COVID_STATUS_HANDLER_H

#include "io/menu.h"
#include "io/input_handler.h"

#include <array>
#include <functional>
#include <string_view>

using CURL = void;

// clang-format off
enum APIType : std::uint8_t { 
    Countries,
    Cities
};
// clang-format on

class covid_status_handler final {
  public:
    using SortFunction = std::function<bool(io::menu::page_type const &,
                                            io::menu::page_type const &)>;

    static constexpr std::array<char const *, 2> apis{
        "https://www.trackcorona.live/api/countries",
        "https://www.trackcorona.live/api/cities"};

    /**
     *  @brief  Constructor.
     *  @param  menu  A menu reference.
     *  @param  input_handler  An input_handler reference.
     *  @param  country An alpha-2-code country code.
     *  @param  sort_fun A sort function pointer.
     */
    explicit covid_status_handler(io::menu &menu,
                                  io::input_handler &input_handler,
                                  std::string_view country,
                                  SortFunction &&sort_fun);

    /**
     *  @brief  Destructor.
     */
    ~covid_status_handler();

    /**
     *  @brief  Specifies the API URL.
     *  @param  api_type    The API Type.
     */
    void set_mode(APIType api_type) noexcept;

    /**
     *  @brief  Sets the maximum timeout to wait for a request to finish.
     *  @param timeout  The maximum timeout in seconds before the request should
     * be aborted.
     */
    void set_timeout(long timeout) noexcept;

    /**
     *  @brief  Prepare the Curl Request.
     *  @return  True if successful, otherwise false.
     */
    [[nodiscard]] bool setup() noexcept;

    /**
     *  @brief  Performs a blocking GET request.
     *  @return True if successful, otherwise false.
     */
    [[nodiscard]] bool request() noexcept;

    /**
     *  @brief  Performs an async non-blocking GET request.
     *  @return True if successful, otherwise false.
     */
    [[nodiscard]] bool async_request();

  private:
    /**
     *  @brief  The callback when the data was completely received.
     *  @return True if successful, otherwise false.
     */
    [[nodiscard]] bool handle_data_received();

  private:
    CURL *handle_;
    std::string json_data_;

    io::menu &menu_;
    io::input_handler &input_handler_;

    std::string_view country_;
    SortFunction sort_fun_;
};

#endif // COVID_PI_COVID_STATUS_HANDLER_H