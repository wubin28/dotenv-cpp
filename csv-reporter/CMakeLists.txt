cmake_minimum_required(VERSION 3.14)
project(csv_reporter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# dotenv header-only include path (relative to this CMakeLists.txt)
# ---------------------------------------------------------------------------
set(DOTENV_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include/laserpants/dotenv")

# ---------------------------------------------------------------------------
# Business-logic library  (shared between main executable and tests)
# ---------------------------------------------------------------------------
add_library(csv_reporter_lib STATIC
    csv_parser.cpp
    config.cpp
    reporter.cpp
)

target_include_directories(csv_reporter_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${DOTENV_INCLUDE_DIR}"
)

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(csv_reporter main.cpp)
target_link_libraries(csv_reporter PRIVATE csv_reporter_lib)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(csv_reporter_tests
    test/csv_parser_test.cpp
    test/config_test.cpp
    test/reporter_test.cpp
)

target_link_libraries(csv_reporter_tests PRIVATE
    csv_reporter_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(csv_reporter_tests)
