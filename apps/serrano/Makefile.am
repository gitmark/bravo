bindir = $(prefix)/bravo/0.0.0/bin
bin_PROGRAMS = serrano
serrano_SOURCES = main.cpp
serrano_CPPFLAGS =  -std=c++14 -I$(srcdir)/../../include -I$(prefix)/include
serrano_LDADD = -lpthread -L$(top_builddir)/src -lbravo -L$(prefix)/lib -ltls -lssl -lcrypto  

install-exec-hook:
	ln -s $(prefix)/bravo/0.0.0/bin/serrano $(prefix)/bin/serrano


