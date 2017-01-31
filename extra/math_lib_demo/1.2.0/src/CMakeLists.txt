cmake_minimum_required(VERSION 2.8)
project(math_lib_demo)
set(name math_lib_demo)
set(major 1)
set(minor 2)
set(patch 0)

if (WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
else()
	set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
endif()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)


add_library(${name} SHARED
base.cpp
dllmain.cpp
math_lib_demo.cpp
stack.cpp
)

set(headers
../include/base.h
../include/stack.h
)

SET_TARGET_PROPERTIES(${name} PROPERTIES 
VERSION ${major}.${minor}.${patch}
SOVERSION 0.0.0)

INSTALL(FILES ${headers} DESTINATION include)
install(TARGETS ${name}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	)

	set(lib_version_files 
	${CMAKE_BINARY_DIR}/src/${name}.${major}.lib
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.lib
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.lib)

	set(dll_version_files 
	${CMAKE_BINARY_DIR}/src/${name}.${major}.dll
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.dll
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.dll)
	
install(FILES ${lib_version_files} DESTINATION lib)
install(FILES ${dll_version_files} DESTINATION bin)
	
if(WIN32)
add_custom_command(TARGET ${name} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.lib 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.lib 	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.lib
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.dll ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.dll 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.dll ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.dll 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.dll ${CMAKE_BINARY_DIR}/src/${name}.${major}.dll 
	)
	
endif()


