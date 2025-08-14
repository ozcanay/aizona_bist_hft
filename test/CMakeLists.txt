# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Find GoogleTest (assumes you install it via your package manager or FetchContent)
find_package(GTest CONFIG REQUIRED)

add_executable(aizona_test
    itch_parser_test.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PCAP REQUIRED libpcap)
include_directories(${PCAP_INCLUDE_DIRS})

target_link_libraries(aizona_test
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock 
        GTest::gmock_main
        algocor_warnings
        algocor_options
        aizona_itch
        ${PCAP_LIBRARIES}
)

# Register the test so `ctest` can run it
add_test(NAME AizonaTest COMMAND aizona_test)
