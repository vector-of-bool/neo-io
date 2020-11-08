#pragma once

#include <neo/io/concepts/read_stream.hpp>
#include <neo/io/concepts/write_stream.hpp>

namespace neo {

// clang-format off
template <typename T>
concept read_write_stream = read_stream<T> && write_stream<T>;
// clang-format on

struct proto_read_write_stream {
    proto_read_write_stream()  = delete;
    ~proto_read_write_stream() = delete;

    proto_transfer_result read_some(mutable_buffer) noexcept;
    proto_transfer_result write_some(const_buffer) noexcept;
};

static_assert(read_write_stream<proto_read_write_stream>);

}  // namespace neo
