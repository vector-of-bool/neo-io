#include <neo/io/read.hpp>

#include <neo/io/stream/string.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Read some data") {
    neo::string_stream strm;
    strm.string = "Hello, person";

    std::string dest_buf;
    dest_buf.resize(5);
    auto res = neo::read(strm, neo::mutable_buffer(dest_buf));
    CHECK(res.bytes_transferred == 5);
    CHECK(dest_buf == "Hello");
}
