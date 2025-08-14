cmake_minimum_required(VERSION 3.10)

find_package(benchmark CONFIG REQUIRED)

add_executable(dummy_benchmark
    dummy_benchmark.cpp
)

target_link_libraries(dummy_benchmark
    PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
        algocor_warnings
        algocor_options
)
