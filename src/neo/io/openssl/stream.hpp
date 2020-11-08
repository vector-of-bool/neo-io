#pragma once

#include "./engine.hpp"
#include <neo/io/concepts/stream.hpp>
#include <neo/io/stream/buffers.hpp>
#include <neo/io/write.hpp>

#include <neo/string_io.hpp>

#include <neo/io/config.hpp>

namespace neo::ssl {

template <read_write_stream Inner>
class NEO_IO_OPENSSL_API_ATTR stream {
public:
    using inner_stream_type = std::remove_cvref_t<Inner>;

private:
    wrap_refs_t<Inner> _inner;

    using _bufs_t   = stream_io_buffers<Inner&>;
    using _engine_t = engine<_bufs_t, _bufs_t>;

    _engine_t _eng;

    void _rebind_io() {
        _eng.input().rebind_stream(_inner);
        _eng.output().rebind_stream(_inner);
    }

public:
    stream() = default;

    explicit stream(context& ctx, Inner&& inner)
        : _inner(NEO_FWD(inner))
        , _eng(ctx, _bufs_t{_inner}, _bufs_t{_inner}) {}

    stream(stream&& other) noexcept
        : _inner(NEO_FWD(other._inner))
        , _eng{NEO_FWD(other._eng)} {
        _rebind_io();
    }

    stream& operator=(stream&& other) noexcept {
        _inner = std::move(other._inner);
        _eng   = std::move(other._eng);
        _rebind_io();
        return *this;
    }

    NEO_DECL_UNREF_GETTER(next_layer, _inner);

    void rebind_next_layer(Inner& in) noexcept requires(std::is_reference_v<Inner>) {
        _inner = std::ref(in);
        _rebind_io();
    }

    void connect() { _eng.connect(); }
    void connect(std::error_code& ec) noexcept { _eng.connect(ec); }

    void shutdown() { _eng.shutdown(); }
    void shutdown(std::error_code& ec) noexcept { _eng.shutdown(ec); }

    auto write_some(const_buffer cbuf) noexcept { return _eng.write_some(cbuf); }
    auto read_some(mutable_buffer mbuf) noexcept { return _eng.read_some(mbuf); }
};

template <read_write_stream S>
explicit stream(context&, S &&) -> stream<S>;

}  // namespace neo::ssl
