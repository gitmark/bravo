
find_path(LibreSSL_INCLUDE_DIRS
  NAMES tls.h
  HINTS /usr/local/opt/libressl/include
)

find_library(LibreSSL_crypto
  NAMES libcrypto.a crypto-38.lib crypto.lib
  HINTS /usr/local/opt/libressl/lib
)

find_library(LibreSSL_ssl
NAMES libssl.a ssl-39.lib ssl.lib
  HINTS /usr/local/opt/libressl/lib
)

find_library(LibreSSL_tls
NAMES libtls.a tls-11.lib tls.lib
  HINTS /usr/local/opt/libressl/lib
)

set(LibreSSL_LIBRARIES
${LibreSSL_crypto}
${LibreSSL_ssl}
${LibreSSL_tls}
)

