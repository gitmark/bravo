
find_path(Check_INCLUDE_DIRS
  NAMES check.h
  HINTS c:/check/include
)

find_library(Check_check
  NAMES check
  HINTS c:/check/lib
)

if (WIN32)
find_library(Check_compat
  NAMES compat
  HINTS c:/check/lib
)
endif()

set(Check_LIBRARIES
${Check_check}
${Check_compat}
)



