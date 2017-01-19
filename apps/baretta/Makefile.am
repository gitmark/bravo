bindir = $(prefix)/bravo/0.0.0/bin
bin_PROGRAMS = baretta
baretta_SOURCES = main.cpp
baretta_CPPFLAGS = -std=c++14 -I$(srcdir)/../../include -I$(prefix)/include
baretta_LDADD = -L$(top_builddir)/src -lbravo -L$(prefix)/lib -ltls -lssl -lcrypto  

install-exec-hook:
	ln -s $(prefix)/bravo/0.0.0/bin/baretta $(prefix)/bin/baretta



