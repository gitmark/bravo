cmake_minimum_required(VERSION 2.8)
project(math_lib_demo)
set(name math_lib_demo)
include(version.txt)

if (WIN32)
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
	set (CMAKE_CXX_FLAGS "/DEF:math-lib-demo.def" ${CMAKE_CXX_FLAGS})
else()
	set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
endif()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

if(WIN32)
add_library(${name} SHARED
base.cpp
dllmain.cpp
math_lib_demo.cpp
stack.cpp
)

add_library(${name}_static STATIC
base.cpp
math_lib_demo.cpp
stack.cpp
)

else()
add_library(${name} SHARED
base.cpp
math_lib_demo.cpp
stack.cpp
)

add_library(${name}_static STATIC
base.cpp
math_lib_demo.cpp
stack.cpp
)

endif()

set(headers
../include/base.h
../include/stack.h
)

SET_TARGET_PROPERTIES(${name} PROPERTIES 
VERSION ${major}.${minor}.${patch}
SOVERSION ${major}
MACOSX_RPATH ON
LINK_FLAGS "/DEF:${CMAKE_SOURCE_DIR}/src/math-lib-demo.def")

target_compile_definitions(${name} PUBLIC B_BUILD_LIB=1)	
target_compile_definitions(${name}_static PUBLIC B_BUILD_LIB=1)	

if(NOT WIN32)
SET_TARGET_PROPERTIES(${name}_static PROPERTIES 
VERSION ${major}.${minor}.${patch}
SOVERSION ${major}
OUTPUT_NAME ${name})
endif()


INSTALL(FILES ${headers} DESTINATION include)
install(TARGETS ${name}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(TARGETS ${name}_static
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

if(WIN32)
	set(lib_version_files 
	${CMAKE_BINARY_DIR}/src/${name}.${major}.lib
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.lib
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.lib)
	
install(FILES ${lib_version_files} DESTINATION lib)

set(dll_version_files 
	${CMAKE_BINARY_DIR}/src/${name}.${major}.dll
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.dll
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.dll)

install(FILES ${dll_version_files} DESTINATION bin)

	set(static_lib_version_files 
	${CMAKE_BINARY_DIR}/src/${name}.${major}_static.lib
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}_static.lib
	${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}_static.lib)
	
install(FILES ${static_lib_version_files} DESTINATION lib)

	elseif(APPLE)
	else()
	endif()
	
	
if(WIN32)
add_custom_command(TARGET ${name} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.lib 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.lib 	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.lib
	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.dll ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}.dll 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.dll ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.dll 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}.dll ${CMAKE_BINARY_DIR}/src/${name}.${major}.dll 
	)	
	
	add_custom_command(TARGET ${name}_static POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}_static.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}.${patch}_static.lib 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}_static.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}.${minor}_static.lib 	
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/${name}_static.lib ${CMAKE_BINARY_DIR}/src/${name}.${major}_static.lib)
	elseif(APPLE)
	else()	
endif()


