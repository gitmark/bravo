cmake_minimum_required(VERSION 2.8)
project(baretta)
set(name baretta)

set(CMAKE_MACOSX_RPATH ON)

find_package(Bravo REQUIRED)
find_package(LibreSSL REQUIRED)

include_directories(${Bravo_INCLUDE_DIRS} ${LibreSSL_INCLUDE_DIRS})

if(WIN32)
	set(LINK_LIBS
	ws2_32.lib
	${LibreSSL_LIBRARIES}
	${Bravo_LIBRARIES})
else()
	set (CMAKE_CXX_FLAGS "--std=c++11" ${CMAKE_CXX_FLAGS})
	set(LINK_LIBS
	${Bravo_LIBRARIES}
	${LibreSSL_LIBRARIES})

message(STATUS "Bravo_LIBRARIES " ${Bravo_LIBRARIES})
endif()

if(WIN32)
    add_executable(${name} WIN32 main.cpp)
    target_link_libraries(${name} ${LINK_LIBS})
    set_target_properties(${name} PROPERTIES LINK_FLAGS
        "/SUBSYSTEM:CONSOLE")
elseif(MACOSX)
    add_executable(${name} main.cpp)
    target_link_libraries(${name} ${CMAKE_BINARY_DIR}/src/libbravo1.dylib)
else()    
    add_executable(${name} main.cpp)
    target_link_libraries(${name} ${CMAKE_BINARY_DIR}/src/libbravo1.so)
endif()

add_dependencies(${name} bravo1)

install(TARGETS ${name} DESTINATION bin)

#add_custom_command(TARGET ${name} POST_BUILD
#	COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/bin/${name} ${ORIG_INSTALL_PREFIX}/bin/${name}
#)
