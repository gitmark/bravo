## Process this file with automake to produce Makefile.in

TESTS = check_bravo test_script.sh
check_PROGRAMS = check_bravo
check_bravo_SOURCES = check_bravo.cpp
check_bravo_CPPFLAGS =  -std=c++14 -I$(top_srcdir)/include -I$(prefix)/include -I$(top_srcdir)
#check_bravo_LDADD = $(top_builddir)/src/libbravo.a -L/usr/local/lib -lcheck -lrt -lpthread -lsubunit
check_bravo_LDADD = $(top_builddir)/src/libbravo.a -L/usr/local/lib -lcheck -lpthread


