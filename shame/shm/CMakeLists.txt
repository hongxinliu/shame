cmake_minimum_required(VERSION 3.0)
project(shm)

add_library(shm shm.cc)
target_link_libraries(shm boost_system rt ${PROTOBUF_LIBRARY})

install(TARGETS shm
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES shm.h
        DESTINATION include/shame/shm)