cmake_minimum_required(VERSION 2.8)
project(bravo_tools)
set(name bravo_tools)

#https://stackoverflow.com/questions/27660048/cmake-check-if-mac-os-x-use-apple-or-apple
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

#set(ORIG_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
#set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/bravo/0.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
elseif(UNIX)
endif()

add_subdirectory(src)
add_subdirectory(apps/baretta)
add_subdirectory(apps/serrano)
#add_subdirectory(apps/exporter)
#add_subdirectory(apps/gen_test_code)
#add_subdirectory(tests)

