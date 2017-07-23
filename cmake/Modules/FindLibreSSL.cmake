
find_path(LibreSSL_INCLUDE_DIRS
  NAMES tls.h
  HINTS c:/libressl/include
)

find_library(LibreSSL_crypto
  NAMES libcrypto.a crypto-38.lib crypto.lib
  HINTS c:/libressl/lib
)

find_library(LibreSSL_ssl
NAMES libssl.a ssl-39.lib ssl.lib
  HINTS c:/libressl/lib
)

find_library(LibreSSL_tls
NAMES libtls.a tls-11.lib tls.lib
  HINTS c:/libressl/lib
)

set(LibreSSL_LIBRARIES
${LibreSSL_crypto}
${LibreSSL_ssl}
${LibreSSL_tls}
)

