cmake_minimum_required(VERSION 2.8)
project(math_app_demo)
set(name math_app_demo)

set(math_lib_dir ${CMAKE_SOURCE_DIR}/../../math_lib_demo/1.2.0/install)

include_directories(${math_lib_dir}/include)
link_directories(${math_lib_dir}/lib)

if(WIN32)
	set(LINK_LIBS
	math_lib_demo.lib
	)
else()
	set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
	set(LINK_LIBS
	math_lib_demo)

	set(STATIC_LINK_LIBS
	math_lib_demo.a)
endif()

if(WIN32)
    add_executable(${name} WIN32 main.cpp)
    target_link_libraries(${name} ${LINK_LIBS})
    set_target_properties(${name} PROPERTIES LINK_FLAGS
        "/SUBSYSTEM:CONSOLE")
else()
    add_executable(${name} main.cpp)
    target_link_libraries(${name} ${LINK_LIBS})
    set_property(TARGET ${name} 
PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)

    add_executable(${name}_static main.cpp)
    target_link_libraries(${name}_static ${STATIC_LINK_LIBS})
    set_property(TARGET ${name} 
PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

install(TARGETS ${name} DESTINATION bin)
install(TARGETS ${name}_static DESTINATION bin)


















