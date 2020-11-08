#include "./read_stream.hpp"

#include <neo/test_concept.hpp>

NEO_TEST_CONCEPT(neo::read_stream<neo::proto_read_stream>);
NEO_TEST_CONCEPT(neo::vectored_read_stream<neo::proto_vectored_read_stream>);
