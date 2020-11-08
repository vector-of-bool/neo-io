#include "./error.hpp"

#include <neo/io/config.hpp>

#include <neo/ufmt.hpp>

#if NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)

#include <openssl/err.h>

using namespace neo::ssl;

namespace {

class ssl_error_category : public std::error_category {
    const char* name() const noexcept override { return "neo::ssl"; }
    std::string message(int ec) const noexcept override {
        const char* msg = ::ERR_reason_error_string(ec);
        return msg ? msg : neo::ufmt("[Unknown error {}]", ec);
    }
};

}  // namespace

const std::error_category& neo::ssl::error_category() noexcept {
    static ssl_error_category instance;
    return instance;
}

void neo::ssl::throw_current(std::string_view str) {
    throw std::system_error(std::error_code(::ERR_get_error(), error_category()), std::string(str));
}

#endif