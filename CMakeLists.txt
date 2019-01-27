cmake_minimum_required(VERSION 2.8)
project(sesc)

add_subdirectory(src)
add_executable(sesc)
target_link_libraries(sesc ${_LIBRARIES})
