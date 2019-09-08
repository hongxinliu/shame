cmake_minimum_required(VERSION 3.0)
project(udpm)

add_library(udpm socket.cc udpm.cc)
target_link_libraries(udpm boost_system pthread)

install(TARGETS udpm
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES socket.h udpm.h
        DESTINATION include/shame/udpm)
