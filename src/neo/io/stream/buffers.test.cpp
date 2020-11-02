#include <neo/io/stream/buffers.hpp>

#include <neo/io/stream/dynbuf.hpp>
#include <neo/io/stream/file.hpp>

#include <neo/buffer_sink.hpp>
#include <neo/fixed_dynamic_buffer.hpp>
#include <neo/shifting_dynamic_buffer.hpp>
#include <neo/test_concept.hpp>

#include <catch2/catch.hpp>

#include <string_view>

NEO_TEST_CONCEPT(neo::buffer_sink<neo::stream_io_buffers<neo::proto_write_stream>>);
NEO_TEST_CONCEPT(neo::buffer_source<neo::stream_io_buffers<neo::proto_read_stream>>);

TEST_CASE("Create a buffer wrapper around a stream") {
    std::string        str;
    neo::dynbuf_stream string_stream{neo::as_dynamic_buffer(str)};

    neo::stream_io_buffers buffers{string_stream};

    CHECK(str.size() == 0);
    buffers.prepare(66);
    CHECK(str.size() == 0);
    buffers.commit(55);
    CHECK(str.size() == 55);
}

TEST_CASE("Read from a file") {
    neo::file_stream this_file{__FILE__};

    neo::stream_io_buffers buffers{this_file};

    neo::const_buffer data  = buffers.next(4096);
    auto              sview = std::string_view(data);
    CHECK(sview.find("sview.find(") != sview.npos);
}

TEST_CASE("Read from a file with custom buffers") {
    neo::file_stream this_file{__FILE__};

    std::array<std::byte, 64>    arr;
    neo::fixed_dynamic_buffer    fixed{arr};
    neo::shifting_dynamic_buffer dbuf{fixed, 0};
    neo::stream_io_buffers       bufs{this_file, dbuf};

    // Ask for more bytes than will possibly fit, and we'll get back a best-effort
    CHECK(dbuf.size() == 0);
    auto data = bufs.next(1024);
    CHECK(dbuf.size() == 64);

    auto sview = std::string_view(data);
    CAPTURE(sview);
    CHECK(sview.find("#include") != sview.npos);
}
