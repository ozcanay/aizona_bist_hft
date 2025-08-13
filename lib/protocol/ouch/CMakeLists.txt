# ITCH Protocol Interface Library
add_library(ouch_protocol INTERFACE)

# Link required dependencies
target_link_libraries(ouch_protocol INTERFACE
    quill::quill
    fmt::fmt
    magic_enum::magic_enum
)

# Add include directories
target_include_directories(ouch_protocol INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Set C++ standard requirements (assuming C++20 for magic_enum)
target_compile_features(ouch_protocol INTERFACE cxx_std_20)

# Debug: Print the fmt target properties (remove after debugging)
get_target_property(FMT_INCLUDE_DIRS fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "fmt include directories: ${FMT_INCLUDE_DIRS}")
get_target_property(FMT_LOCATION fmt::fmt LOCATION)
message(STATUS "fmt location: ${FMT_LOCATION}")