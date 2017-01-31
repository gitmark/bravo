cmake_minimum_required(VERSION 2.8)
project(math_app_demo)
set(name math_app_demo)

set(math_lib_dir ${CMAKE_SOURCE_DIR}/../../math_lib_demo/1.1.0/install)

include_directories(${math_lib_dir}/include)
link_directories(${math_lib_dir}/lib)

if(WIN32)
	set(LINK_LIBS
	math_lib_demo.lib
	)
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

install(TARGETS ${name} DESTINATION bin)
