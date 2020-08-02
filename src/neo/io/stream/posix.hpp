#pragma once

#include <neo/io/concepts/stream.hpp>
#include <neo/io/stream/result.hpp>

#include <neo/assert.hpp>
#include <neo/buffer_range.hpp>

#include <utility>

namespace neo {

struct posix_iovec_type {
    void*  iov_base;
    size_t iov_len;
};

class posix_fd_stream {
public:
    // POSIX is all `int` all the way down
    using native_handle_type = int;

    /**
     * The `invalid_native_handle_value` is a sentinel value of the native handle type that
     * represents a "not-a-stream" state. This is the initial value of the native_handle() for all
     * stream objects.
     */
    constexpr static native_handle_type invalid_native_handle_value = -1;

public:
    /**
     * Default-constructs to an invalid stream
     */
    constexpr posix_fd_stream() = default;
    /**
     * Destructor will close the file descriptor
     */
    ~posix_fd_stream() { close(); }

    /**
     * Move-construct a stream
     */
    constexpr posix_fd_stream(posix_fd_stream&& other)
        : _fd(other._fd) {
        other._fd = invalid_native_handle_value;
    }

    /**
     * Move-assign a stream
     */
    posix_fd_stream& operator=(posix_fd_stream&& other) noexcept {
        close();
        _fd = other.release();
        return *this;
    }

    /**
     * Read from the stream
     */
    template <mutable_buffer_range Bufs>
    [[nodiscard]] native_stream_read_result read_some(Bufs out) noexcept {
        return _do_read_some(out);
    }

    /**
     * Write to the stream
     */
    template <buffer_range Bufs>
    [[nodiscard]] native_stream_write_result write_some(Bufs in) noexcept {
        return _do_write_some(in);
    }

    /**
     * Obtain the file descriptor of the stream
     */
    [[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return _fd; }

    /**
     * Close the stream
     */
    void close() noexcept;

    /**
     * Relinquish ownership of the underlying stream, and return the file descriptor to the caller
     */
    [[nodiscard]] constexpr native_handle_type release() noexcept {
        auto r = _fd;
        _fd    = invalid_native_handle_value;
        return r;
    }

    /**
     * Adopt ownership over the given file descriptor object. The file descriptor must be given as
     * an rvalue-reference, which will be set to the invalid file descriptor value.
     */
    [[nodiscard]] static posix_fd_stream from_native_handle(native_handle_type&& fd) noexcept {
        posix_fd_stream ret;
        ret._fd = fd;
        fd      = invalid_native_handle_value;
        return ret;
    }

    /**
     * Duplicate an existing file descriptor and return a stream that owns the new file descriptor.
     */
    [[nodiscard]] static posix_fd_stream dup_native_handle(native_handle_type fd);

private:
    // For decent performance in the case of a lot of small buffers, we have a thread-local array of
    // iovec objects that can are set to refer to the buffers in a given buffer sequence, and then
    // vectored IO is executed against that array. The array is stored in a separate TU, so we just
    // declare a pointer here.
    static std::size_t                    _tl_iov_arr_len;
    static thread_local posix_iovec_type* _tl_small_iov_array;

    native_handle_type _fd = invalid_native_handle_value;

    native_stream_write_result _do_writev(std::size_t nbufs) noexcept;
    native_stream_read_result  _do_readv(std::size_t nbufs) noexcept;

    native_stream_write_result _do_write_some(const_buffer buf) noexcept;
    native_stream_write_result _do_write_some(mutable_buffer buf) noexcept {
        return _do_write_some(const_buffer(buf));
    }
    native_stream_read_result _do_read_some(mutable_buffer buf) noexcept;

    template <typename T>
    T _mk_result(ssize_t n_transfered) {
        if (n_transfered == -1) {
            return {0, errno};
        }
        neo_assert(invariant, n_transfered >= 0, "Did not transfer any data??", n_transfered);
        return {static_cast<std::size_t>(n_transfered), 0};
    }

    /**
     * Prepare the thread-local iovec array to refer to the elements of the
     * given buffer sequence.
     * @returns The number of elements in the iovec array.
     * @note this may not consume all elements of the buffer sequence, since
     * the thread-local iovec array is statically sized and there is a limit
     * to the number of elements we may insert into it.
     */
    template <typename T>
    std::size_t _prep_iovec(T&& bufs) noexcept {
        std::size_t n_bufs = 0;
        using std::begin;
        using std::end;
        auto buf_iter = begin(bufs);
        auto buf_stop = end(bufs);
        auto iov_it   = _tl_small_iov_array;
        auto iov_end  = _tl_small_iov_array + _tl_iov_arr_len;
        for (; iov_it != iov_end && buf_iter != buf_stop; ++iov_it, ++buf_iter, ++n_bufs) {
            auto buf         = *buf_iter;
            iov_it->iov_base = const_cast<std::byte*>(buf.data());
            iov_it->iov_len  = buf.size();
        }
        return n_bufs;
    }

    template <mutable_buffer_range T>
    native_stream_read_result _do_read_some(T&& bufs) noexcept {
        auto n_bufs = _prep_iovec(bufs);
        return _do_readv(n_bufs);
    }

    template <buffer_range T>
    native_stream_write_result _do_write_some(T&& bufs) noexcept {
        auto n_bufs = _prep_iovec(bufs);
        return _do_writev(n_bufs);
    }
};

static_assert(read_write_stream<posix_fd_stream>);

using native_stream = posix_fd_stream;

}  // namespace neo
