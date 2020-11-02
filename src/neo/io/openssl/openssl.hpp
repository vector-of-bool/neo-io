#pragma once

#if !__has_include(<openssl/ssl.h>)
#error "This file may only be included if OpenSSL is available"
#endif

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>

#define NEO_IO_OPENSSL_VERSION_AT_LEAST(Major, Minor, Patch)                                       \
    (OPENSSL_VERSION_NUMBER >= ((Major * 0x1'00'00'000) + (Minor * 0x1'00'000) + (Patch * 0x1'000)))
