
find_path(LibreSSL_INCLUDE_DIRS
  NAMES tls.h
  HINTS c:/libressl/include
)

find_library(LibreSSL_crypto
  NAMES libcrypto.a crypto.lib
  HINTS c:/libressl/lib
)

find_library(LibreSSL_ssl
NAMES libssl.a ssl.lib
  HINTS c:/libressl/lib
)

find_library(LibreSSL_tls
NAMES libtls.a tls.lib
  HINTS c:/libressl/lib
)

set(LibreSSL_LIBRARIES
${LibreSSL_tls}
${LibreSSL_ssl}
${LibreSSL_crypto}
)

