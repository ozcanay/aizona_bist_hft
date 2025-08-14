# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Find GoogleTest (assumes you install it via your package manager or FetchContent)
find_package(GTest CONFIG REQUIRED)

add_executable(dummy_test
    dummy_test.cpp
)

target_link_libraries(dummy_test
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock 
        GTest::gmock_main
        algocor_warnings
        algocor_options
)

# Register the test so `ctest` can run it
add_test(NAME DummyTest COMMAND dummy_test)
