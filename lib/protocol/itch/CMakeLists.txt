# ITCH Protocol Interface Library
add_library(aizona_itch STATIC itch_parser.cpp)

# Link required dependencies
target_link_libraries(aizona_itch PUBLIC
    quill::quill
    fmt::fmt
    magic_enum::magic_enum
)

# Add include directories
target_include_directories(aizona_itch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)