cmake_minimum_required(VERSION 2.8)
project(test_bravo_string_utils_header)
set(name test_bravo_string_utils_header)

if (WIN32)
else()
set (CMAKE_CXX_FLAGS "--std=c++14" ${CMAKE_CXX_FLAGS})
endif()

find_package(Bravo REQUIRED)
find_package(Check REQUIRED)
find_package(LibreSSL REQUIRED)

include_directories(${Bravo_INCLUDE_DIRS} ${LibreSSL_INCLUDE_DIRS} ${Check_INCLUDE_DIRS})
if(WIN32)
set(LINK_LIBS
${Check_LIBRARIES}
ws2_32.lib
${LibreSSL_LIBRARIES}
${Bravo_LIBRARIES}
)
elseif(APPLE)
set(LINK_LIBS
${Bravo_LIBRARIES}
${LibreSSL_LIBRARIES}
${Check_LIBRARIES})
else()
set(LINK_LIBS
${Bravo_LIBRARIES}
${LibreSSL_LIBRARIES}
${Check_LIBRARIES}
rt
subunit
pthread)
endif()



if(WIN32)
	add_executable(${name} WIN32 test_bravo_string_utils_header.cpp)
	target_link_libraries(${name} ${LINK_LIBS})
	set_target_properties(${name} PROPERTIES LINK_FLAGS
       "/SUBSYSTEM:CONSOLE")
else()
	add_executable(${name} test_bravo_string_utils_header.cpp)
	target_link_libraries(${name} ${LINK_LIBS})
endif()

add_dependencies(${name} bravo)

install(TARGETS ${name}  DESTINATION bin)

if(WIN32)
	add_custom_target(check test_bravo_string_utils_header.exe)
else()
	add_custom_target(check test_bravo_string_utils_header)
endif()







