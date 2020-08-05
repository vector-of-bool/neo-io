#pragma once

#include <neo/io/stream/result.hpp>

#include <neo/const_buffer.hpp>
#include <neo/mutable_buffer.hpp>

namespace neo {

class win32_handle_stream_base {
protected:
    // No one can use this class directly
    ~win32_handle_stream_base() = default;

    /**
     * Win32 uses void* as HANDLE
     */
    using native_handle_type = void*;

    /**
     * The `invalid_native_handle_value` is a sentinel value of the native handle type that
     * represents a "not-a-stream" state. This is the initial value of the native_handle() for all
     * stream objects.
     */
    inline const static native_handle_type invalid_native_handle_value
        = reinterpret_cast<void*>(~static_cast<std::ptrdiff_t>(0));

    native_handle_type _native_handle = invalid_native_handle_value;

    void close() noexcept;

    native_stream_read_result  read_some(neo::mutable_buffer) noexcept;
    native_stream_write_result write_some(neo::const_buffer) noexcept;
};

#if _WIN32
using native_stream_base = win32_handle_stream_base;
#endif

}  // namespace neo
