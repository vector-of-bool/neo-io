#include "./init.hpp"

#if NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

using namespace neo::ssl;

openssl_app_init::openssl_app_init() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

openssl_app_init::~openssl_app_init() {
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
}

#endif
