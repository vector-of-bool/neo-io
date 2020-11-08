#include <neo/config-pp.hpp>

#if __has_include(<neo/io.tweaks.hpp>)
#include <neo/io.tweaks.hpp>
#endif

#ifndef neo_io_ToggleFeature_OpenSSL_Support
#if __has_include(<openssl/conf.h>)
#define neo_io_ToggleFeature_OpenSSL_Support Enabled
#else
#define neo_io_ToggleFeature_OpenSSL_Support Disabled
#endif
#endif

#if NEO_FeatureIsDisabled(neo_io, OpenSSL_Support)
#define NEO_IO_OPENSSL_API_ATTR [[deprecated("OpenSSL support is not enabled/available")]]
#else
#define NEO_IO_OPENSSL_API_ATTR
#endif
