#include <neo/io/write.hpp>

#include <neo/io/stream/stdio.hpp>
#include <neo/io/stream/string.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Write some data") {
    neo::string_stream strm;
    auto               res = neo::write(strm, neo::const_buffer("Hello!\n"));
    CHECK(res.bytes_transferred == 7);
    CHECK(strm.string == "Hello!\n");
    strm.string.clear();

    res = neo::write(strm, neo::const_buffer("Hello!\n"), neo::transfer_exactly{3});
    CHECK(res.bytes_transferred == 3);
    CHECK(strm.string == "Hel");
}

TEST_CASE("Write a buffer sequence") {
    neo::string_stream strm;
    auto               bufs = {
        neo::const_buffer("Hello, "),
        neo::const_buffer("world!"),
    };
    auto res = neo::write(strm, bufs);
    CHECK(res.bytes_transferred == 13);
    CHECK(strm.string == "Hello, world!");
}
