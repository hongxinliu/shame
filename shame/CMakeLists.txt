cmake_minimum_required(VERSION 3.0)
project(shame)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

add_subdirectory(udpm)
add_subdirectory(shm)
add_subdirectory(examples)

add_library(shame shame.cc)
target_link_libraries(shame udpm shm)

add_executable(shame_server shame_server.cc)
target_link_libraries(shame_server boost_system rt pthread)

install(TARGETS shame shame_server
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES shame.h subscription.h shame_data.h
        DESTINATION include/shame)
