exec_prefix	= ${prefix}/bravo/0.0.0
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib

bin_PROGRAMS = serrano
serrano_SOURCES = main.cpp
serrano_CPPFLAGS = -std=c++14 -I$(srcdir)/../../include -I$(prefix)/include
serrano_LDADD = -lpthread -L$(top_builddir)/src -lbravo -L$(prefix)/lib -ltls -lssl -lcrypto  

install-exec-hook:
	ln -sf $(exec_prefix)/bin/serrano $(prefix)/bin/serrano


