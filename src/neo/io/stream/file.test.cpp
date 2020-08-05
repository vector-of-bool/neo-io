#include <neo/io/stream/file.hpp>

#include <neo/io/concepts/stream.hpp>

#include <neo/test_concept.hpp>

#include <catch2/catch.hpp>

NEO_TEST_CONCEPT(neo::read_write_stream<neo::file_stream>);
