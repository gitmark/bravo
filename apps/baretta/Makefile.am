bindir = $(prefix)/bravo/0.0.1/bin
bin_PROGRAMS = baretta
baretta_SOURCES = main.cpp
baretta_CPPFLAGS = -std=c++14 -I$(srcdir)/../../include -I$(prefix)/include
baretta_LDFLAGS = -L$(prefix)/lib -lcrypto -lssl -ltls -L../../src -lbravo

TESTS = test_version

install-exec-hook:
	ln -s $(bindir)/baretta $(prefix)/bin/baretta
 