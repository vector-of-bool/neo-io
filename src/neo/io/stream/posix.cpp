#ifndef _WIN32

#include "./posix.hpp"

#include <neo/io/stream/stdio.hpp>

#include <sys/uio.h>
#include <unistd.h>

#include <climits>
#include <cstdio>

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
thread_local posix_iovec_type* posix_fd_stream::_tl_small_iov_array = tl_small_iov_array.data();

posix_fd_stream posix_fd_stream::dup_native_handle(posix_fd_stream::native_handle_type fd) {
    native_handle_type new_fd = ::dup(fd);
    if (new_fd >= 0) {
        return from_native_handle(std::move(new_fd));
    }
    throw std::system_error(std::error_code(errno, std::system_category()), "::dup() failed");
}

void posix_fd_stream::close() noexcept {
    if (_fd != invalid_native_handle_value) {
        auto failed = ::close(_fd);
        if (failed) {
            std::fputs("posix_fd_stream::close() encountered an error()\n", stderr);
        }
    }
}

native_stream_write_result posix_fd_stream::_do_writev(std::size_t n_bufs) noexcept {
    auto iov_ptr  = reinterpret_cast<const iovec*>(_tl_small_iov_array);
    auto nwritten = ::writev(_fd, iov_ptr, static_cast<int>(n_bufs));
    return _mk_result<native_stream_write_result>(nwritten);
}

native_stream_write_result posix_fd_stream::_do_write_some(const_buffer buf) noexcept {
    auto nwritten = ::write(_fd, buf.data(), buf.size());
    return _mk_result<native_stream_write_result>(nwritten);
}

native_stream_read_result posix_fd_stream::_do_read_some(mutable_buffer buf) noexcept {
    auto nread = ::read(_fd, buf.data(), buf.size());
    return _mk_result<native_stream_read_result>(nread);
}

namespace {

struct nonown_fd_stream : neo::posix_fd_stream {
    ~nonown_fd_stream() { this->_fd = invalid_native_handle_value; }
    nonown_fd_stream(native_handle_type fd) { _fd = fd; }
};

static nonown_fd_stream stdin_fd_stream{STDIN_FILENO};
static nonown_fd_stream stdout_fd_stream{STDOUT_FILENO};
static nonown_fd_stream stderr_fd_stream{STDERR_FILENO};

}  // namespace

posix_fd_stream& neo::stdin_stream  = stdin_fd_stream;
posix_fd_stream& neo::stdout_stream = stdout_fd_stream;
posix_fd_stream& neo::stderr_stream = stderr_fd_stream;

#endif
