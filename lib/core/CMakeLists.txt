# ITCH Protocol Interface Library
add_library(aizona_core STATIC orderbook_builder.cpp)

# Link required dependencies
target_link_libraries(aizona_core PUBLIC
    quill::quill
    fmt::fmt
    magic_enum::magic_enum
    aizona_utility
)

# Add include directories
target_include_directories(aizona_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)