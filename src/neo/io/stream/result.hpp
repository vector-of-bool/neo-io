#pragma once

#include <neo/io/concepts/result.hpp>

#include <system_error>

namespace neo {

struct native_transfer_result {
    std::size_t    bytes_transferred = 0;
    int            errn              = 0;
    constexpr bool has_error() const noexcept { return errn != 0; }
    auto           error() const noexcept { return std::error_code(errn, std::system_category()); }
};

struct native_stream_read_result : native_transfer_result {};
struct native_stream_write_result : native_transfer_result {};

static_assert(transfer_result<native_stream_read_result>);
static_assert(transfer_result<native_stream_write_result>);

}  // namespace neo
