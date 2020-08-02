#pragma once

#include <neo/io/concepts/result.hpp>

#include <system_error>

namespace neo {

struct native_stream_read_result {
    std::size_t bytes_transferred = 0;
    int         error             = 0;
    bool        is_failure() const noexcept { return bool(error); }

    native_stream_read_result& operator+=(native_stream_read_result o) noexcept {
        bytes_transferred += o.bytes_transferred;
        error = o.error;
        return *this;
    }
};

struct native_stream_write_result {
    std::size_t bytes_transferred = 0;
    int         error             = 0;
    bool        is_failure() const noexcept { return bool(error); }

    native_stream_write_result& operator+=(native_stream_write_result o) noexcept {
        bytes_transferred += o.bytes_transferred;
        error = o.error;
        return *this;
    }
};

static_assert(transfer_result<native_stream_read_result>);
static_assert(transfer_result<native_stream_write_result>);

}  // namespace neo
