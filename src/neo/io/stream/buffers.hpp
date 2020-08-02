#pragma once

#include <neo/io/concepts/stream.hpp>
#include <neo/io/read.hpp>
#include <neo/io/write.hpp>

#include <neo/as_dynamic_buffer.hpp>
#include <neo/bytes.hpp>
#include <neo/fwd.hpp>
#include <neo/io_buffer.hpp>
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
    using buffers_type = std::remove_cvref_t<Stream>;

private:
    wrap_if_reference_t<Stream>  _strm;
    wrap_if_reference_t<Buffers> _bufs;

public:
    constexpr stream_io_buffers() = default;

    constexpr explicit stream_io_buffers(Stream&& s)
        : _strm(NEO_FWD(s)) {}

    constexpr stream_io_buffers(Stream&& s, Buffers&& bufs)
        : _strm(NEO_FWD(s))
        , _bufs(NEO_FWD(bufs)) {}

    constexpr auto& stream() noexcept { return unref(_strm); }
    constexpr auto& stream() const noexcept { return unref(_strm); }
    constexpr auto& buffers() noexcept { return unref(_bufs); }
    constexpr auto& buffers() const noexcept { return unref(_bufs); }

    constexpr decltype(auto) prepare(std::size_t size) requires(write_stream<stream_type>) {
        auto ready_size = buffers().size();
        if (ready_size < size) {
            safe_grow_dynbuf(buffers(), size - ready_size);
        }
        return buffers().data(0, size);
    }

    constexpr decltype(auto) commit(std::size_t size) requires(write_stream<stream_type>) {
        auto out_bufs  = buffers().data(0, size);
        auto write_res = write(stream(), out_bufs);
        buffers().consume(write_res.bytes_transferred);
    }

    constexpr decltype(auto) data(std::size_t size) requires(read_stream<stream_type>) {
        auto already_size = buffers().size();
        if (already_size < size) {
            auto want_grow_n = size - already_size;
            auto in_bufs     = safe_grow_dynbuf(buffers(), want_grow_n);
            auto read_res    = read(stream(), in_bufs);
            auto unused_size = buffer_size(in_bufs) - read_res.bytes_transferred;
            buffers().shrink(unused_size);
        }
        return buffers().data(0, buffers().size());
    }

    constexpr decltype(auto) consume(std::size_t size) requires(read_stream<stream_type>) {
        buffers().consume(size);
    }
};

template <typename Stream>
stream_io_buffers(Stream &&) -> stream_io_buffers<Stream>;

template <typename Stream, typename Buffers>
stream_io_buffers(Stream&&, Buffers &&) -> stream_io_buffers<Stream, Buffers>;

}  // namespace neo
