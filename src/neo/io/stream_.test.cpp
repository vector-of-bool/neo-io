#include <neo/io/stream.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Write to stdout") {
    // Write a simple string
    auto res = neo::stdout_stream.write_some(neo::const_buffer("nope\n"));
    CHECK(res.bytes_transferred > 0);
    CHECK_FALSE(res.is_failure());
}
