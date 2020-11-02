#pragma once

#include <string_view>
#include <system_error>

#include <neo/io/config.hpp>

namespace neo::ssl {

/**
 * @brief Obtain the error_category for OpenSSL
 *
 * @return NEO_IO_OPENSSL_API_ATTR const&
 */
NEO_IO_OPENSSL_API_ATTR
const std::error_category& error_category() noexcept;

/**
 * @brief Throw a system_error with the current OpenSSL error information.
 */
NEO_IO_OPENSSL_API_ATTR [[noreturn]] void throw_current(std::string_view);

}  // namespace neo::ssl