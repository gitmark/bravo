cmake_minimum_required(VERSION 2.8)
project(baretta)
set(name baretta)

find_package(Bravo REQUIRED)
find_package(LibreSSL REQUIRED)

include_directories(${Bravo_INCLUDE_DIRS} ${LibreSSL_INCLUDE_DIRS})

if(WIN32)
	set(LINK_LIBS
	ws2_32.lib
	${LibreSSL_LIBRARIES}
	${Bravo_LIBRARIES})
else()
	set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
	set(LINK_LIBS
	${Bravo_LIBRARIES}
	${LibreSSL_LIBRARIES})
endif()

if(WIN32)
    add_executable(${name} WIN32 main.cpp)
    target_link_libraries(${name} ${LINK_LIBS})
    set_target_properties(${name} PROPERTIES LINK_FLAGS
        "/SUBSYSTEM:CONSOLE")
else()
    add_executable(${name} main.cpp)
    target_link_libraries(${name} ${LINK_LIBS})
endif()

add_dependencies(${name} bravo)

install(TARGETS ${name} DESTINATION bin)
