cmake_minimum_required(VERSION 2.8)
project(bravo)
set(name bravo)

if (WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
else()
	set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
endif()

find_package(Bravo)
find_package(LibreSSL)

include_directories(${Bravo_INCLUDE_DIRS} ${LibreSSL_INCLUDE_DIRS})

add_library(${name}
argv_parser.cpp
base_socket.cpp
content_types.cpp
hex.cpp
http_listen_port.cpp
http_message.cpp
http_utils.cpp
norm_socket.cpp
socket_utils.cpp
ssl_socket.cpp
tls_socket.cpp
url.cpp
vec_buf.cpp)

install(TARGETS ${name} ARCHIVE DESTINATION lib)

add_custom_command(TARGET ${name} POST_BUILD
	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/lib/libbravo.a ${ORIG_INSTALL_PREFIX}/lib/libbravo.a
	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/include/bravo ${ORIG_INSTALL_PREFIX}/include/bravo
)
