#include <neo/io/openssl/engine.hpp>
#include <neo/io/stream/buffers.hpp>
#include <neo/io/stream/socket.hpp>

#include <neo/io/concepts/stream.hpp>
#include <neo/string_io.hpp>
#include <neo/test_concept.hpp>

#include <catch2/catch.hpp>

#if NEO_FeatureIsEnabled(neo_io, OpenSSL_Support)

#include <csignal>

TEST_CASE("Initialize a connection") {
    neo::ssl::openssl_app_init init;

    auto gh_addr = neo::address::resolve("github.com", "80");
    auto sock    = neo::socket::open_connected(gh_addr, neo::socket::type::stream);
    auto ctx     = neo::ssl::context{neo::ssl::protocol::tls_any, neo::ssl::role::client};

    neo::stream_io_buffers input{sock};
    neo::stream_io_buffers output{sock};

    neo::ssl::engine eng{ctx, input, output};
    // CHECK_THROWS_AS(eng.connect(), std::system_error);
    input.io_buffers()  = {};
    output.io_buffers() = {};

    gh_addr = neo::address::resolve("github.com", "443");
    sock    = neo::socket::open_connected(gh_addr, neo::socket::type::stream);
    eng     = {ctx, input, output};
    REQUIRE_NOTHROW(eng.connect());

#if defined(SIGPIPE)
    std::signal(SIGPIPE, SIG_IGN);
#endif

    neo::write(eng,
               neo::const_buffer("HEAD /vector-of-bool/neo-buffer/archive/0.4.2.tar.gz HTTP/1.1\r\n"
                                 "Host: github.com\r\n"
                                 "Content-Length: 0\r\n\r\n"));
    std::string buf;
    buf.resize(1024);
    eng.read_some(neo::mutable_buffer(buf));
    INFO(buf);
    eng.shutdown();
}

NEO_TEST_CONCEPT(
    neo::read_write_stream<neo::ssl::engine<neo::proto_buffer_source, neo::proto_buffer_sink>>);

#endif
