cmake_minimum_required(VERSION 2.8)

include( ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Toolchain-cross-m32.cmake)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libsuc)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libmint)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libcore)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libapp)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libll)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libmem)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libnet)

add_definitions(-DSIMICS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Other flags
option(ATOMIC "Atomic" TRUE)

add_subdirectory(libapp)
add_subdirectory(libcore)
#add_subdirectory(libemul)
#add_subdirectory(libll)
#add_subdirectory(libmem)
#add_subdirectory(libmint)
#add_subdirectory(libnet)
#add_subdirectory(libpint)
#add_subdirectory(libpower)
#add_subdirectory(librst)
#add_subdirectory(libsescspot)
#add_subdirectory(libsesctherm)
#add_subdirectory(libsmp)
#add_subdirectory(libsuc)
#add_subdirectory(libsuperlu)
#add_subdirectory(libtm)
#add_subdirectory(libvmem)
#add_subdirectory(misc)
