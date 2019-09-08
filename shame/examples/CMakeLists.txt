cmake_minimum_required(VERSION 3.0)
project(examples)

add_subdirectory(proto)

add_executable(talker_raw talker_raw.cc)
target_link_libraries(talker_raw shame)

add_executable(talker_proto talker_proto.cc)
target_link_libraries(talker_proto shame examples_proto)

add_executable(listener_raw listener_raw.cc)
target_link_libraries(listener_raw shame)

add_executable(listener_proto listener_proto.cc)
target_link_libraries(listener_proto shame examples_proto)

install(TARGETS talker_raw talker_proto listener_raw listener_proto
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
