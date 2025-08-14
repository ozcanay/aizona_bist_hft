# ITCH Protocol Interface Library
add_library(aizona_utility STATIC quill_wrapper.cpp thread.cpp)

# Link required dependencies
target_link_libraries(aizona_utility PUBLIC
    quill::quill
    fmt::fmt
    magic_enum::magic_enum
)

# Add include directories
target_include_directories(aizona_utility PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)