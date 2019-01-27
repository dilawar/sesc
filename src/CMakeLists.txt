cmake_minimum_required(VERSION 2.6)

# Setup compiler flags depending on OS.
if(UNIX)
    add_definitions( -DSIMICS )
endif(UNIX)

macro(ADD_LIBRARY_FROM_SUBDIR libname)
    message(STATUS "Adding library ${libname}" )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/${libname} )
    file(GLOB C_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${libname}/*.c" )
    file(GLOB CPP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${libname}/*.cpp" )
    list(LENGTH SRC SRC_LENGTH)
    if(SRC_LENGTH GREATER 0)
        add_library( ${libname} STATIC ${SRC} )
    else()
        message( STATUS "No source file found for ${libname}. Ignoring ...")
    endif()
endmacro()

set(SUBDIRS libapp libcore libemul liblibapp.a libll libmem libmint libnet
        libpint libpower librst libsescspot libsesctherm libsmp libsuc
        libsuperlu libtm libvmem
        )

foreach( _dir ${SUBDIRS})
    ADD_LIBRARY_FROM_SUBDIR( ${_dir} )
endforeach()
