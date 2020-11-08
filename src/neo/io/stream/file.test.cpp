#include <neo/io/stream/file.hpp>

#include <neo/io/concepts/stream.hpp>
#include <neo/io/read.hpp>

#include <neo/test_concept.hpp>

#include <catch2/catch.hpp>

NEO_TEST_CONCEPT(neo::read_write_stream<neo::file_stream>);

TEST_CASE("Open a file") {
    neo::file_stream file("test.txt", neo::open_mode::write);
    file.write_some(neo::const_buffer("Hello, text!"));
    file.close();

    file = neo::file_stream::open("test.txt");
    std::string rbuf;
    rbuf.resize(20);
    auto nread = neo::read(file, neo::mutable_buffer(rbuf));
    rbuf.resize(nread.bytes_transferred);
    CHECK(rbuf == "Hello, text!");
}
