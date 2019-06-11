cmake_minimum_required(VERSION 2.8)
file(GLOB SRCS *.c)
add_library(app STATIC ${SRCS})
