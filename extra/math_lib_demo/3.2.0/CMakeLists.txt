cmake_minimum_required(VERSION 2.8)
project(math_lib_demo)
set(name math_lib_demo)

if(WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
elseif(UNIX)
endif()

add_subdirectory(src)
