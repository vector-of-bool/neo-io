#pragma once

#include <neo/io/concepts.hpp>
#include <neo/io/stream/result.hpp>

#include <neo/buffer_algorithm.hpp>
#include <neo/dynamic_buffer.hpp>

namespace neo {

/**
 * Adapt a dynamic buffer object to act as a read/write stream.
 *
 * Reading from the stream will pull data from the beginning of the buffer, and
 * writing will append data to the end of the buffer.
 */
template <dynamic_buffer DynBuffer>
class dynamic_buffer_stream {
    DynBuffer _bufs;

public:
    dynamic_buffer_stream(DynBuffer&& b)
        : _bufs(NEO_FWD(b)) {}

    template <buffer_range Bufs>
    native_stream_write_result write_some(const Bufs& src) noexcept {
        const auto grow_size = buffer_size(src);
        auto       out       = _bufs.grow(grow_size);
        return {buffer_copy(out, src)};
    }

    template <mutable_buffer_range Bufs>
    native_stream_read_result read_some(const Bufs& dest) noexcept {
        const std::size_t avail_size = _bufs.size();
        const std::size_t read_size  = (std::min)(buffer_size(dest), avail_size);
        buffer_copy(dest, _bufs.data(0, read_size));
        _bufs.consume(read_size);
        return {read_size};
    }
};

}  // namespace neo
