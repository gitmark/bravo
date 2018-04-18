# on windows build with ...
# ....\builds\bravo>..\..\projects\scripts\vcvars.bat
# ....\builds\bravo>cmake -G "Visual Studio 14 2015 Win64" ../../projects/bravo

cmake_minimum_required(VERSION 2.8)
project(bravo1)
set(name bravo1)

if (APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
endif (APPLE)

if (WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE -DBRAVO_LIB_BUILD)
else()
	set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
endif()

find_package(Bravo)
find_package(LibreSSL)

include_directories(${Bravo_INCLUDE_DIRS} ${LibreSSL_INCLUDE_DIRS})

add_library(${name} SHARED
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
vec_buf.cpp
cpp_types.cpp)

if(WIN32)
  target_link_libraries(${name} ws2_32 ${LibreSSL_LIBRARIES})
install(TARGETS ${name} ARCHIVE DESTINATION lib)

else()
  target_link_libraries(${name} ${LibreSSL_LIBRARIES})
install(TARGETS ${name} LIBRARY DESTINATION bravo/brv1.0.0/lib)
INSTALL (
    DIRECTORY ${CMAKE_SOURCE_DIR}/include
    DESTINATION bravo/brv1.0.0
    FILES_MATCHING PATTERN "*.h*")
endif()
#add_custom_command(TARGET ${name} POST_BUILD
#	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bravo/brv1.0.0 ${CMAKE_INSTALL_PREFIX}/bravo/brv1.0
#	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bravo/brv1.0 ${CMAKE_INSTALL_PREFIX}/bravo/brv1
#	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bravo/brv1/lib/libbravo1.dylib ${CMAKE_INSTALL_PREFIX}/lib/libbravo1.dylib
#	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bravo/brv1/lib/libbravo1.dylib ${CMAKE_INSTALL_PREFIX}/lib/libbravo.dylib
#	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bravo/brv1/include/bravo ${CMAKE_INSTALL_PREFIX}/include/bravo
#)
