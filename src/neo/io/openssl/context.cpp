#include "./context.hpp"

#include "./error.hpp"
#include <system_error>

#if NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)

#if !__has_include(<openssl/conf.h>)
#error                                                                                             \
    "The OpenSSL headers must be available in order to compile with OpenSSL support (Did not find <openssl/conf.h>)"
#else

#include "./openssl.hpp"

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>

using namespace neo::ssl;

#define MY_CTX_PTR (static_cast<::SSL_CTX*>(_ssl_ctx_ptr))

context::context(protocol pr, role rl) {
    using p = protocol;
    using r = role;

    auto unsupported = [](std::string s) {
        throw std::system_error(make_error_code(std::errc::protocol_not_supported), s);
    };

#define OpenSSL_1_1_0_Or_Greater NEO_IO_OPENSSL_VERSION_AT_LEAST(1, 1, 0)

/// Load a new OpenSSL context with the method with the given 'Prefix'
#define LOAD_METHOD(Prefix)                                                                        \
    ::SSL_CTX_new(rl == r::server ? NEO_CONCAT(Prefix, _server_method)()                           \
                                  : rl == r::client ? NEO_CONCAT(Prefix, _client_method)()         \
                                                    : NEO_CONCAT(Prefix, _method)())

/// Set the min/max protocol version to the version specified by 'Version'
#define SET_PROTO_VERSION(Var, Version)                                                            \
    do {                                                                                           \
        if (Var) {                                                                                 \
            ::SSL_CTX_set_min_proto_version(Var, Version);                                         \
            ::SSL_CTX_set_max_proto_version(Var, Version);                                         \
        }                                                                                          \
    } while (0)

    switch (pr) {

    case p::ssl_v2:
#if OpenSSL_1_1_0_Or_Greater || defined(OPENSSL_NO_SSL2)
        unsupported("This OpenSSL does not support SSLv2");
#else
        _ssl_ctx_ptr = LOAD_METHOD(SSLv2);
#endif
        break;

    case p::ssl_v3:
#if OpenSSL_1_1_0_Or_Greater
        _ssl_ctx_ptr = LOAD_METHOD(TLS);
        SET_PROTO_VERSION(MY_CTX_PTR, SSL3_VERSION);
#elif defined(OPENSSL_NO_SSL3)
        unsupported("This OpenSSL does not support SSLv3");
#else  // Older OpenSSL uses versioned method funcs
        _ssl_ctx_ptr = LOAD_METHOD(SSLv3);
#endif
        break;

    case p::tls_v1:
#if OpenSSL_1_1_0_Or_Greater
        _ssl_ctx_ptr = LOAD_METHOD(TLS);
        SET_PROTO_VERSION(MY_CTX_PTR, TLS1_VERSION);
#elif defined(SSL_TXT_TLSV1)
        _ssl_ctx_ptr = LOAD_METHOD(TLSv1);
#else
        unsupported("This OpenSSL does not support TLSv1");
#endif
        break;

    case p::tls_v1_1:
#if OpenSSL_1_1_0_Or_Greater
        _ssl_ctx_ptr = LOAD_METHOD(TLSv1);
        SET_PROTO_VERSION(MY_CTX_PTR, TLS1_1_VERSION);
#elif defined(SSL_TXT_TLSV1_1)
        _ssl_ctx_ptr = LOAD_METHOD(TLSv1_1);
#else
        unsupported("This OpenSSL does not support TLSv1.1");
#endif
        break;

    case p::tls_v1_2:
#if OpenSSL_1_1_0_Or_Greater
        _ssl_ctx_ptr = LOAD_METHOD(TLS);
        SET_PROTO_VERSION(MY_CTX_PTR, TLS1_2_VERSION);
#elif defined(SSL_TXT_TLSV1_2)
        _ssl_ctx_ptr = LOAD_METHOD(TLSv1_2);
#else
        unsupported("This OpenSSL does not support TLSv1.2");
#endif
        break;

    case p::tls_v1_3:
#if NEO_IO_OPENSSL_VERSION_AT_LEAST(1, 1, 1)
        _ssl_ctx_ptr = LOAD_METHOD(TLS);
        SET_PROTO_VERSION(MY_CTX_PTR, TLS1_3_VERSION);
#else
        unsupported("This OpenSSL does not support TLSv1.3");
#endif
        break;

    case p::ssl_any:
        _ssl_ctx_ptr = LOAD_METHOD(SSLv23);
        break;

    case p::tls_any:
#if OpenSSL_1_1_0_Or_Greater
        _ssl_ctx_ptr = LOAD_METHOD(TLS);
        if (_ssl_ctx_ptr) {
            ::SSL_CTX_set_min_proto_version(MY_CTX_PTR, TLS1_VERSION);
        }
#else
        _ssl_ctx_ptr = LOAD_METHOD(SSLv23);
        if (_ssl_ctx_ptr) {
            ::SSL_CTX_set_options(MY_CTX_PTR, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
        }
#endif
        break;
    }

    if (_ssl_ctx_ptr == nullptr) {
        throw std::system_error(std::error_code(static_cast<int>(::ERR_get_error()),
                                                error_category()),
                                "Failed to allocate SSL protocol method");
    }
}

void context::_close() {
    if (_ssl_ctx_ptr) {
        ::SSL_CTX_free(MY_CTX_PTR);
    }
    _ssl_ctx_ptr = nullptr;
}

#endif
#endif
