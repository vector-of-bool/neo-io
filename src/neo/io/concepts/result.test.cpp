#include "./result.hpp"

#include <neo/test_concept.hpp>

NEO_TEST_CONCEPT(neo::transfer_result<neo::proto_transfer_result>);
NEO_TEST_CONCEPT(neo::transfer_result<neo::basic_transfer_result>);
