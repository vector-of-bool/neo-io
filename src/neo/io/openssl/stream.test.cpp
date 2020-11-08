#include "./stream.hpp"

#include <neo/io/stream/socket.hpp>

#include <neo/test_concept.hpp>

#include <catch2/catch.hpp>

#if NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)

NEO_TEST_CONCEPT(neo::read_write_stream<neo::ssl::stream<neo::proto_read_write_stream>>);

TEST_CASE("Create an OpenSSL stream") {
    auto sock = neo::socket::open_connected(neo::address::resolve("www.google.com", "443"),
                                            neo::socket::type::stream);
    neo::ssl::openssl_app_init ssl_init;
    neo::ssl::context          ctx{neo::ssl::protocol::tls_any, neo::ssl::role::client};
    auto                       ssl  = neo::ssl::stream{ctx, std::move(sock)};
    auto                       ssl2 = std::move(ssl);
    ssl2.connect();

    ssl2.write_some(
        neo::const_buffer("GET / HTTP/1.1\r\n"
                          "Host: www.google.com\r\n"
                          "Content-Length: 0\r\n"
                          "\r\n"));
    std::string buf;
    buf.resize(1024);
    auto rc = ssl2.read_some(neo::as_buffer(buf));
    INFO(rc.error().message());
    REQUIRE_FALSE(rc.error());
    CHECK(buf.starts_with("HTTP/1.1"));
}

#endif
