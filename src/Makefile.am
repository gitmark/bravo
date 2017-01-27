exec_prefix	= ${prefix}/bravo/0.0.0
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib

lib_LIBRARIES = libbravo.a
libbravo_a_SOURCES = argv_parser.cpp base_socket.cpp content_types.cpp hex.cpp http_listen_port.cpp http_message.cpp http_utils.cpp norm_socket.cpp socket_utils.cpp ssl_socket.cpp tls_socket.cpp url.cpp vec_buf.cpp
include_HEADERS = $(top_srcdir)/include/bravo/*.h
libbravo_a_CPPFLAGS = -std=c++14 -I$(top_srcdir)/include -I$(prefix)/include

install-exec-hook:
	ln -sf $(exec_prefix)/lib/libbravo.a $(prefix)/lib/libbravo.a
	ln -sf $(exec_prefix)/include/bravo $(prefix)/include/bravo
 
