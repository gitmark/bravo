libdir = $(prefix)/bravo/0.0.1/lib
lib_LIBRARIES = libbravo.a
libbravo_a_SOURCES = argv_parser.cpp base_listen_port.cpp base_socket.cpp content_types.cpp hex.cpp http_listen_port.cpp http_message.cpp http_utils.cpp norm_socket.cpp socket_utils.cpp ssl_socket.cpp tls_socket.cpp url.cpp vec_buf.cpp
includedir = $(prefix)/bravo/0.0.1/include/bravo
include_HEADERS = $(srcdir)/../include/bravo/*.h
libbravo_a_CPPFLAGS = -std=c++14 -I$(srcdir)/../include -I$(prefix)/include

install-exec-hook:
	ln -s $(libdir)/libbravo.a $(prefix)/lib/libbravo.a
	ln -s $(includedir) $(prefix)/include/bravo
 