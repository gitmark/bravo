
find_path(Check_INCLUDE_DIRS
  NAMES check.h
  HINTS c:/check/include
)

find_library(Check_LIBRARIES
  NAMES check
  HINTS c:/check/lib
)



