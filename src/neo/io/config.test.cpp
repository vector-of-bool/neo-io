#include "./config.hpp"

static_assert(NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)
              ^ NEO_FeatureIsDisabled(neo_io, OpenSSL_Support));