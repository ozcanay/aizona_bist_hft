# ITCH Protocol Interface Library
add_library(aizona_client STATIC market_data_client.cpp order_entry_client.cpp)

# Link required dependencies
target_link_libraries(aizona_client PUBLIC
    quill::quill
    fmt::fmt
    magic_enum::magic_enum
    aizona_utility
    aizona_network
)

# Add include directories
target_include_directories(aizona_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)