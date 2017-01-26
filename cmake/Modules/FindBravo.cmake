if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/include AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/apps/baretta)
	set(Bravo_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
else()
	find_path(Bravo_INCLUDE_DIRS
	NAMES bravo/string_utils.h
	)
endif()

if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/src AND IS_DIRECTORY 	${CMAKE_SOURCE_DIR}/apps/baretta)
    if (WIN32)
		set(Bravo_LIBRARIES ${CMAKE_BINARY_DIR}/src/bravo.lib)
    else()
        set(Bravo_LIBRARIES ${CMAKE_BINARY_DIR}/src/libbravo.a)
    endif()
else()
    find_library(Bravo_LIBRARIES
    NAMES bravo)
endif()

