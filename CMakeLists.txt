cmake_minimum_required(VERSION 3.10)

project(hft_engine LANGUAGES CXX)

find_package(quill CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include(cmake/StandardProjectSettings.cmake)

# Warnings
add_library(algocor_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
option(WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
algocor_set_project_warnings(
    algocor_warnings
    ${WARNINGS_AS_ERRORS})

# Sanitizers
option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ON)
option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
option(ENABLE_SANITIZER_UNDEFINED "Enable UB sanitizer" OFF)
option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
include(cmake/Sanitizers.cmake)
add_library(algocor_options INTERFACE)
algocor_enable_sanitizers(
    algocor_options
    ${ENABLE_SANITIZER_ADDRESS}
    ${ENABLE_SANITIZER_LEAK}
    ${ENABLE_SANITIZER_UNDEFINED}
    ${ENABLE_SANITIZER_THREAD}
    ${ENABLE_SANITIZER_MEMORY})

# Static analyzers
include(cmake/StaticAnalyzers.cmake)
option(algocor_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
if(algocor_ENABLE_CLANG_TIDY)
    message(STATUS "Enabling clang-tidy")
    algocor_enable_clang_tidy(algocor_options ${WARNINGS_AS_ERRORS})
endif()
option(algocor_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
if(algocor_ENABLE_CPPCHECK)
    message(STATUS "Enabling cppcheck")
    algocor_enable_cppcheck(${WARNINGS_AS_ERRORS})
endif()

add_subdirectory(src)
add_subdirectory(lib)
# add_subdirectory(test)
