#pragma once

#include <neo/io/concepts/result.hpp>

#include <neo/buffer_range.hpp>
#include <neo/const_buffer.hpp>
#include <neo/ref.hpp>

namespace neo {

// clang-format off
template <typename T, typename Bufs = const_buffer>
concept write_stream = requires(T stream, Bufs buf) {
    { stream.write_some(buf) } noexcept -> transfer_result;
};

template <typename T>
concept vectored_write_stream =
    write_stream<T> &&
    write_stream<T, proto_buffer_range>;
// clang-format on

template <write_stream Stream, typename Bufs = const_buffer>
requires write_stream<Stream, Bufs>  //
    using write_result_t = std::decay_t<decltype(ref_v<Stream>.write_some(cref_v<Bufs>))>;

struct proto_write_stream {
    proto_write_stream()  = delete;
    ~proto_write_stream() = delete;

    proto_transfer_result write_some(const_buffer) noexcept;
};

struct proto_vectored_write_stream {
    proto_vectored_write_stream()  = delete;
    ~proto_vectored_write_stream() = delete;

    template <buffer_range R>
    proto_transfer_result write_some(R&&) noexcept;
};

static_assert(write_stream<proto_write_stream>);
static_assert(vectored_write_stream<proto_vectored_write_stream>);

}  // namespace neo
