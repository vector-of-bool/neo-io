#include "./posix.hpp"

#ifndef _WIN32

#include <neo/io/stream/stdio.hpp>

#include <sys/uio.h>
#include <unistd.h>

#include <climits>
#include <cstdio>
#include <iostream>

using namespace neo;

namespace {

/// Do a sanity check to see if our own definition of `iovec` is binary-compatible
/// with the one defined by the system headers.
static_assert(sizeof(posix_iovec_type) == sizeof(iovec)
                  && offsetof(posix_iovec_type, iov_base) == offsetof(iovec, iov_base)
                  && offsetof(posix_iovec_type, iov_len) == offsetof(iovec, iov_len),
              "The size and/or alignment of this system's iovec type does not match our baked-in "
              "expectation. This means that we haven't considered the iovec for your platform. "
              "Please file a bug report!");

/// The number of iovec elements is defined by IOV_MAX, but we don't want to make a ridiculously
/// large static array in case it is too large. Cap the length to 16 elements.
constexpr int tl_iov_arr_len = (std::min)(IOV_MAX, 16);

std::array<posix_iovec_type, tl_iov_arr_len> tl_small_iov_array;

}  // namespace

/// Initialize the class-thread_local pointer to the iovec array
thread_local posix_iovec_type* posix_fd_stream_base::_tl_small_iov_array
    = tl_small_iov_array.data();

void posix_fd_stream_base::close() noexcept {
    if (_native_handle != invalid_native_handle_value) {
        auto failed = ::close(_native_handle);
        if (failed) {
            std::fputs("posix_fd_stream_base::close() encountered an error()\n", stderr);
        }
        _native_handle = invalid_native_handle_value;
    }
}

native_stream_write_result posix_fd_stream_base::_do_writev(std::size_t n_bufs) noexcept {
    auto iov_ptr  = reinterpret_cast<const iovec*>(_tl_small_iov_array);
    auto nwritten = ::writev(_native_handle, iov_ptr, static_cast<int>(n_bufs));
    return _mk_result<native_stream_write_result>(nwritten);
}

native_stream_write_result posix_fd_stream_base::_do_write_some(const_buffer buf) noexcept {
    auto nwritten = ::write(_native_handle, buf.data(), buf.size());
    return _mk_result<native_stream_write_result>(nwritten);
}

native_stream_read_result posix_fd_stream_base::_do_read_some(mutable_buffer buf) noexcept {
    auto nread = ::read(_native_handle, buf.data(), buf.size());
    return _mk_result<native_stream_read_result>(nread);
}

namespace {

struct nonown_fd_stream : neo::native_stream {
    ~nonown_fd_stream() { (void)this->release(); }
    nonown_fd_stream(native_handle_type fd) { reset(std::move(fd)); }
};

static nonown_fd_stream stdin_fd_stream{STDIN_FILENO};
static nonown_fd_stream stdout_fd_stream{STDOUT_FILENO};
static nonown_fd_stream stderr_fd_stream{STDERR_FILENO};

}  // namespace

neo::native_stream& neo::stdin_stream  = stdin_fd_stream;
neo::native_stream& neo::stdout_stream = stdout_fd_stream;
neo::native_stream& neo::stderr_stream = stderr_fd_stream;

#endif
