#pragma once

#ifdef _WIN32
#include <neo/io/stream/win32.hpp>
#else
#include <neo/io/stream/posix.hpp>
#endif

#include <neo/io/concepts/stream.hpp>

namespace neo {

class native_stream : native_stream_base {
public:
    using native_stream_base::invalid_native_handle_value;
    using native_stream_base::native_handle_type;

    using native_stream_base::close;
    using native_stream_base::read_some;
    using native_stream_base::write_some;

public:
    /**
     * Default-constructs to an invalid stream
     */
    constexpr native_stream() = default;
    /**
     * Destructor will close the underlying stream
     */
    ~native_stream() { close(); }

    constexpr explicit native_stream(native_handle_type&& handle) noexcept {
        this->_native_handle = handle;
        handle               = this->invalid_native_handle_value;
    }

    constexpr native_stream(native_stream&& other) noexcept
        : native_stream(std::move(other._native_handle)) {}

    native_stream& operator=(native_stream&& other) noexcept {
        reset(other.release());
        return *this;
    }

    constexpr native_handle_type native_handle() const noexcept { return this->_native_handle; }

    /**
     * Relinquish ownership of the underlying stream, and return the handle to the caller
     */
    [[nodiscard]] constexpr native_handle_type release() noexcept {
        auto r               = native_handle();
        this->_native_handle = this->invalid_native_handle_value;
        return r;
    }

    /**
     * Replace the underlying handle with the given handle. An existing open stream will
     * be closed.
     */
    void reset(native_handle_type&& hndl) noexcept {
        close();
        this->_native_handle = hndl;
        hndl                 = this->invalid_native_handle_value;
    }

    /**
     * Adopt ownership over the given handle object. The handle must be given as
     * an rvalue-reference, which will be set to the invalid handle value.
     */
    [[nodiscard]] static native_stream from_native_handle(native_handle_type&& hndl) noexcept {
        native_stream ret;
        ret.reset(std::move(hndl));
        return ret;
    }
};

using native_stream_handle = native_stream::native_handle_type;

using native_read_result  = read_result_t<native_stream>;
using native_write_result = write_result_t<native_stream>;

}  // namespace neo
