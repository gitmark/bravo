cmake_minimum_required(VERSION 2.8)
project(bravo_tools)
set(name bravo_tools)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
elseif(UNIX)
endif()

add_subdirectory(src)
add_subdirectory(apps/baretta)
add_subdirectory(apps/serrano)
add_subdirectory(tests)

