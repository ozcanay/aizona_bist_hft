add_executable(AYDIN trading_client.cpp)
target_link_libraries(AYDIN PRIVATE itch_protocol)  # This is crucial!

get_target_property(AYDIN_INCLUDE_DIRS AYDIN INCLUDE_DIRECTORIES)
get_target_property(AYDIN_LINK_LIBS AYDIN LINK_LIBRARIES)
message(STATUS "AYDIN include dirs: ${AYDIN_INCLUDE_DIRS}")
message(STATUS "AYDIN link libraries: ${AYDIN_LINK_LIBS}")