cmake_minimum_required(VERSION 3.0)
project(shame)

add_subdirectory(udpm)
add_subdirectory(shm)

add_library(shame shame.cc
            $<TARGET_OBJECTS:udpm>
            $<TARGET_OBJECTS:shm>)
target_link_libraries(shame ${PROTOBUF_LIBRARY} boost_system rt pthread)

add_executable(shame_server shame_server.cc)
target_link_libraries(shame_server boost_system rt pthread)

install(TARGETS shame shame_server
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

file(GLOB HDRS *.h)
install(FILES ${HDRS} DESTINATION include/shame)
