#pragma once

#include <neo/io/concepts/result.hpp>

#include <neo/buffer_range.hpp>
#include <neo/mutable_buffer.hpp>
#include <neo/ref.hpp>

namespace neo {

// clang-format off
template <typename T, typename Bufs = mutable_buffer>
concept read_stream = requires(T stream, Bufs buf) {
    { stream.read_some(buf) } noexcept -> transfer_result;
};

template <typename T>
concept vectored_read_stream =
    read_stream<T> &&
    read_stream<T, proto_mutable_buffer_range>;
// clang-format on

/**
 * @brief Obtain the transfer_result type of the given read_stream, as if by
 * passing it an instance of 'Bufs'.
 *
 * @tparam Stream A read_stream.
 * @tparam Bufs A mutable_buffer_range type that can be passed to the .read_some() of the stream
 */
template <read_stream Stream, typename Bufs = mutable_buffer>
requires read_stream<Stream, Bufs>  //
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
