#exec_prefix	= ${prefix}/bravo/0.0.0
#bindir = ${exec_prefix}/bin
#libdir = ${exec_prefix}/lib

bin_PROGRAMS = baretta
baretta_SOURCES = main.cpp
baretta_CPPFLAGS = -std=c++14 -I$(srcdir)/../../include -I$(prefix)/include
baretta_LDADD = -L$(top_builddir)/src -lbravo -L$(prefix)/lib -ltls -lssl -lcrypto  

#install-exec-hook:
#	ln -sf $(exec_prefix)/bin/baretta $(prefix)/bin/baretta



