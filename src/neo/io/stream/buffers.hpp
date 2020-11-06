#pragma once

#include <neo/io/concepts/stream.hpp>
#include <neo/io/read.hpp>
#include <neo/io/write.hpp>

#include <neo/as_dynamic_buffer.hpp>
#include <neo/buffer_sink.hpp>
#include <neo/buffer_source.hpp>
#include <neo/bytes.hpp>
#include <neo/dynbuf_io.hpp>
#include <neo/fwd.hpp>
#include <neo/ref.hpp>
#include <neo/shifting_dynamic_buffer.hpp>

namespace neo {

template <typename Stream,
          dynamic_buffer Buffers
          = shifting_dynamic_buffer<dynamic_buffer_byte_container_adaptor<std::string>>>
requires(read_stream<Stream> || write_stream<Stream>)  //
    class stream_io_buffers {
public:
    using stream_type  = std::remove_cvref_t<Stream>;
    using buffers_type = std::remove_cvref_t<Buffers>;

private:
    wrap_if_reference_t<Stream> _strm;
    dynbuf_io<Buffers>          _io_bufs;

public:
    constexpr stream_io_buffers() = default;

    constexpr explicit stream_io_buffers(Stream&& s)
        : _strm(NEO_FWD(s)) {}

    constexpr stream_io_buffers(Stream&& s, dynbuf_io<Buffers> io_bufs)
        : _strm(NEO_FWD(s))
        , _io_bufs(NEO_FWD(io_bufs)) {}

    constexpr stream_io_buffers(Stream&& s, Buffers&& bufs)
        : _strm(NEO_FWD(s))
        , _io_bufs(NEO_FWD(bufs)) {}

    constexpr auto& stream() noexcept { return unref(_strm); }
    constexpr auto& stream() const noexcept { return unref(_strm); }
    constexpr auto& io_buffers() noexcept { return _io_bufs; }
    constexpr auto& io_buffers() const noexcept { return _io_bufs; }

    constexpr decltype(auto) prepare(std::size_t size) requires(write_stream<stream_type>) {
        return _io_bufs.prepare(size);
    }

    constexpr decltype(auto) commit(std::size_t size) requires(write_stream<stream_type>) {
        _io_bufs.commit(size);
        auto write_res = write(stream(), _io_bufs.next(size));
        _io_bufs.consume(write_res.bytes_transferred);
        throw_if_transfer_errant(write_res, "Failed write in stream_io_buffers::commit()");
    }

    constexpr decltype(auto) next(std::size_t size) requires(read_stream<stream_type>) {
        if (_io_bufs.available() < size) {
            auto want_read_n = size - _io_bufs.available();
            auto in_bufs     = _io_bufs.prepare(want_read_n);
            auto read_res    = stream().read_some(in_bufs);
            _io_bufs.commit(read_res.bytes_transferred);
            throw_if_transfer_errant(read_res, "Failed read in stream_io_buffers::next()");
        }
        return _io_bufs.next(size);
    }

    constexpr decltype(auto) consume(std::size_t size) requires(read_stream<stream_type>) {
        _io_bufs.consume(size);
    }
};

template <typename Stream>
stream_io_buffers(Stream &&) -> stream_io_buffers<Stream>;

template <typename Stream, typename Buffers>
stream_io_buffers(Stream&&, Buffers &&) -> stream_io_buffers<Stream, Buffers>;

}  // namespace neo
