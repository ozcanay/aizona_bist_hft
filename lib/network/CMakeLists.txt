add_library(aizona_network STATIC tcp_client.cpp tcp_socket.cpp udp_socket.cpp)

# Link required dependencies
target_link_libraries(aizona_network PUBLIC
    quill::quill
)

# Add include directories
target_include_directories(aizona_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)