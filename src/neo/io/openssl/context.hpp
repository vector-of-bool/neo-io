#pragma once

#include "./init.hpp"

#include <neo/io/config.hpp>

#include <neo/utility.hpp>

namespace neo::ssl {

/**
 * @brief The SSL/TLS protocol versions available.
 */
enum class protocol {
    ssl_v2,
    ssl_v3,
    tls_v1,
    tls_v1_1,
    tls_v1_2,
    tls_v1_3,
    ssl_any,
    tls_any,
};

/**
 * @brief The expected role of the SSL/TLS engine.
 */
enum class role {
    both,
    client,
    server,
};

/**
 * @brief Represents an OpenSSL API context. All engines are created against a context.
 */
class NEO_IO_OPENSSL_API_ATTR context {
    void* _ssl_ctx_ptr = nullptr;

    void _close();

public:
    explicit context(protocol, role = role::both);
    ~context() { _close(); }

    context(context&& o) noexcept
        : _ssl_ctx_ptr(neo::take(o._ssl_ctx_ptr)) {}

    context& operator=(context&& c) noexcept {
        _close();
        _ssl_ctx_ptr = neo::take(c._ssl_ctx_ptr);
        return *this;
    }

    void*       c_ptr() noexcept { return _ssl_ctx_ptr; }
    const void* c_ptr() const noexcept { return _ssl_ctx_ptr; }
};

}  // namespace neo::ssl
