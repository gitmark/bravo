## Process this file with automake to produce Makefile.in

if LINUX
LINUX_FLAGS = -lrt -lsubunit
endif

BRAVO_CPPFLAGS =  -std=c++14 -I$(top_srcdir)/include -I$(prefix)/include -I$(top_srcdir)
BRAVO_LDADD = $(top_builddir)/src/libbravo.a -L/usr/local/lib -lcheck -lpthread $(LINUX_FLAGS)

TESTS = test_bravo_string_utils_trim test_bravo_string_utils_header test_script.sh
check_PROGRAMS = test_bravo_string_utils_trim test_bravo_string_utils_header

test_bravo_string_utils_trim_SOURCES = test_bravo_string_utils_trim.cpp
test_bravo_string_utils_trim_CPPFLAGS =  $(BRAVO_CPPFLAGS)
test_bravo_string_utils_trim_LDADD = $(BRAVO_LDADD)

test_bravo_string_utils_header_SOURCES = test_bravo_string_utils_header.cpp
test_bravo_string_utils_header_CPPFLAGS =  $(BRAVO_CPPFLAGS)
test_bravo_string_utils_header_LDADD = $(BRAVO_LDADD)

