#include "./win32.hpp"

#ifdef _WIN32

#include <neo/io/stream/stdio.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace neo;

void win32_handle_stream_base::close() noexcept {
    if (_native_handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(_native_handle);
        _native_handle = invalid_native_handle_value;
    }
}

native_read_result win32_handle_stream_base::read_some(mutable_buffer mbuf) noexcept {
    DWORD n_did_read = 0;
    auto  dw_size    = static_cast<DWORD>(mbuf.size());
    ::SetLastError(0);
    ::ReadFile(_native_handle, mbuf.data(), dw_size, &n_did_read, nullptr);
    return {{
        .bytes_transferred = static_cast<std::size_t>(n_did_read),
        .errn              = static_cast<int>(::GetLastError()),
    }};
}

native_write_result win32_handle_stream_base::write_some(const_buffer cbuf) noexcept {
    DWORD n_did_write = 0;
    auto  dw_size     = static_cast<DWORD>(cbuf.size());
    ::SetLastError(0);
    ::WriteFile(_native_handle, cbuf.data(), dw_size, &n_did_write, nullptr);
    return {{
        .bytes_transferred = static_cast<std::size_t>(n_did_write),
        .errn              = static_cast<int>(::GetLastError()),
    }};
}

namespace {

struct nonown_handle_stream : neo::native_stream {
    ~nonown_handle_stream() { (void)this->release(); }
    nonown_handle_stream(native_handle_type hndl) { reset(std::move(hndl)); }
};

static nonown_handle_stream stdin_fd_stream{::GetStdHandle(STD_INPUT_HANDLE)};
static nonown_handle_stream stdout_fd_stream{::GetStdHandle(STD_OUTPUT_HANDLE)};
static nonown_handle_stream stderr_fd_stream{::GetStdHandle(STD_ERROR_HANDLE)};

}  // namespace

neo::native_stream& neo::stdin_stream  = stdin_fd_stream;
neo::native_stream& neo::stdout_stream = stdout_fd_stream;
neo::native_stream& neo::stderr_stream = stderr_fd_stream;

#endif
