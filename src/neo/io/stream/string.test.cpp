#include <neo/io/stream/string.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Basic string streaming") {
    neo::string_stream strm;
    CHECK(strm.string.empty());

    strm.write_some(neo::const_buffer("Hello, person!"));
    CHECK(strm.string == "Hello, person!");

    strm.write_some(neo::const_buffer(" More text."));
    CHECK(strm.string == "Hello, person! More text.");
}
