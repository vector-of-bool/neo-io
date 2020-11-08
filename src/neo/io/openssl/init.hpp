#pragma once

#include <neo/io/config.hpp>

namespace neo::ssl {

/**
 * @brief Initialize OpenSSL for the current application for the scope of this
 * object.
 *
 * This object should be created and outlive the scope of all of the
 * application's ssl::context objects.
 */
class NEO_IO_OPENSSL_API_ATTR openssl_app_init {
public:
    openssl_app_init();
    ~openssl_app_init();
};

}  // namespace neo::ssl
