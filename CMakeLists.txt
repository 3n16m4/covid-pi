cmake_minimum_required(VERSION 3.13)

project(covid-pi C CXX)
include(cmake/StandardProjectSettings.cmake)

set(CMAKE_CXX_STANDARD 17)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)

# Very basic PCH example
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if (ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which
  # is a good idea if any #define's change
  # 
  # consider breaking this out per project as necessary 
  target_precompile_headers(project_options INTERFACE <vector> <string> <map> <utility>)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
        ssd1306_i2c/oled_fonts.h
        ssd1306_i2c/ssd1306_i2c.h
        ssd1306_i2c/ssd1306_i2c.c

        include/utils.h

        include/io/input_handler.h
        include/io/menu.h
        include/io/oled_display.h

        include/json/covid_data.h

        src/io/input_handler.cpp
        src/io/menu.cpp
        src/io/oled_display.cpp

        src/main.cpp)


set (WPI_PATH third_party/WiringPi/wiringPi)
include_directories (include ${WPI_PATH})
find_library(WPI_LIB wiringPi HINTS libs NO_CMAKE_FIND_ROOT_PATH)
if(NOT WPI_LIB)
    message(FATAL_ERROR "wiringPi library not found")
endif()

set (CURL_PATH third_party/curl)
include_directories(include ${CURL_PATH}/include)
find_library(CURL_LIB curl HINTS libs NO_CMAKE_FIND_ROOT_PATH)
if(NOT CURL_LIB)
    message(FATAL_ERROR "curl library not found")
endif()

find_library(CRYPTO_LIB crypto HINTS libs NO_CMAKE_FIND_ROOT_PATH)
if(NOT CRYPTO_LIB)
    message(FATAL_ERROR "crypto library not found")
endif()

find_library(SSL_LIB ssl HINTS libs NO_CMAKE_FIND_ROOT_PATH)
if(NOT SSL_LIB)
    message(FATAL_ERROR "ssl library not found")
endif()

find_library(Z_LIB z HINTS libs NO_CMAKE_FIND_ROOT_PATH)
if(NOT Z_LIB)
    message(FATAL_ERROR "z library not found")
endif()

find_package(Threads REQUIRED)

add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(rapidjson)
add_subdirectory(cxxopts)

include_directories(rapidjson/include)
include_directories(cxxopts/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        project_options
        #project_warnings
        fmt::fmt-header-only
        ${CURL_LIB}
        ${CRYPTO_LIB}
        ${SSL_LIB}
        ${Z_LIB}
        ${WPI_LIB}
        ${CMAKE_THREAD_LIBS_INIT})
