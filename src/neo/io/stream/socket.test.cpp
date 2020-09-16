#include <neo/io/stream/socket.hpp>

#include <neo/io/read.hpp>
#include <neo/io/write.hpp>

#include <neo/string_io.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Open a connected socket") {
    CHECK_THROWS_AS(neo::socket::
                        open_connected(neo::address::resolve("google.com.this.tld.does.not.exist",
                                                             "80"),
                                       neo::socket::type::stream),
                    std::system_error);

    auto sock = neo::socket::open_connected(neo::address::resolve("www.google.com", "80"),
                                            neo::socket::type::stream);
    auto req  = neo::const_buffer(
        "HEAD / HTTP/1.1\r\n"
        "Host: www.google.com\r\n"
        "\r\n");
    neo::write(sock, req);

    neo::string_dynbuf_io strbuf;
    auto                  res = sock.read_some(strbuf.prepare(1024));
    CHECK_FALSE(res.is_failure());
    CHECK(res.bytes_transferred > 0);
    strbuf.commit(res.bytes_transferred);
    CHECK(strbuf.read_area_view().substr(0, 8) == "HTTP/1.1");
}
