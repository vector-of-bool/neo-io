#pragma once

#include <neo/io/concepts/result.hpp>

#include <neo/buffer_range.hpp>
#include <neo/mutable_buffer.hpp>
#include <neo/ref.hpp>

namespace neo {

// clang-format off
template <typename T>
concept read_stream = requires(T stream, mutable_buffer buf) {
    { stream.read_some(buf) } noexcept -> transfer_result;
};

template <typename T>
concept vectored_read_stream =
    read_stream<T> &&
    requires (T stream, proto_mutable_buffer_range rng) {
        { stream.read_some(rng) } noexcept
            -> transfer_result;
    };
// clang-format on

template <read_stream Stream, typename Bufs = mutable_buffer>
using read_result_t = std::decay_t<decltype(ref_v<Stream>.read_some(cref_v<Bufs>))>;

struct proto_read_stream {
    proto_read_stream()  = delete;
    ~proto_read_stream() = delete;

    proto_transfer_result read_some(mutable_buffer) noexcept;
};

struct proto_vectored_read_stream {
    proto_vectored_read_stream()  = delete;
    ~proto_vectored_read_stream() = delete;

    template <mutable_buffer_range S>
    proto_transfer_result read_some(S&&) noexcept;
};

}  // namespace neo
