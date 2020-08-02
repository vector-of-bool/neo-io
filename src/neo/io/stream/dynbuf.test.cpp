#include <neo/io/stream/dynbuf.hpp>

#include <neo/as_dynamic_buffer.hpp>

#include <catch2/catch.hpp>

#include <string>

template <neo::write_stream>
static void check_write_stream() {}

template <neo::read_stream>
static void check_read_stream() {}

TEST_CASE("Write into a string") {
    check_write_stream<neo::dynamic_buffer_stream<neo::proto_dynamic_buffer>>();

    std::string                str;
    neo::dynamic_buffer_stream strm(neo::as_dynamic_buffer(str));
    auto                       res = strm.write_some(neo::const_buffer("Hello, world!"));
    CHECK_FALSE(res.is_failure());
    CHECK(res.bytes_transferred == 13);
    CHECK(str == "Hello, world!");

    strm.write_some(neo::const_buffer(" I am a string."));
    CHECK(str == "Hello, world! I am a string.");
}

TEST_CASE("Read from a string") {
    check_read_stream<neo::dynamic_buffer_stream<neo::proto_dynamic_buffer>>();

    std::string                str = "Hello, buffers!";
    neo::dynamic_buffer_stream strm(neo::as_dynamic_buffer(str));

    std::string other_str;
    other_str.resize(5);
    auto res = strm.read_some(neo::mutable_buffer(other_str));
    CHECK(!res.is_failure());
    CHECK(res.bytes_transferred == other_str.size());
    // The prefix will have been removed from the string
    CHECK(str == ", buffers!");
    CHECK(other_str == "Hello");
}
