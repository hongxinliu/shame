set(TARGET_NAME udpm)

file(GLOB HDRS *.h)
file(GLOB SRCS *.cc)

add_library(${TARGET_NAME} OBJECT ${SRCS})

install(FILES ${HDRS} DESTINATION include/shame/udpm)
