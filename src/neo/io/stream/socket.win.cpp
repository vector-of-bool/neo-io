#include "./socket.hpp"

#if NEO_OS_IS_WINDOWS

#include <WinSock2.h>

using namespace neo;

void io_detail::init_winsock() noexcept {
    WSADATA wsd;
    ::WSAStartup(MAKEWORD(2, 2), &wsd);
}

void io_detail::wsa_socket_stream::shutdown() {
    if (_native_handle != INVALID_SOCKET) {
        ::shutdown(native_handle(), SD_BOTH);
    }
}

native_stream_write_result
io_detail::wsa_socket_stream::_do_write_some(const_buffer cbuf) noexcept {
    ::WSABUF buf{.len = static_cast<ULONG>(cbuf.size()),
                 .buf = reinterpret_cast<CHAR*>(const_cast<std::byte*>(cbuf.data()))};

    DWORD nsent = 0;
    ::SetLastError(0);
    ::WSASend(native_handle(), &buf, 1, &nsent, 0, nullptr, nullptr);
    return {{.bytes_transferred = nsent, .errn = ::WSAGetLastError()}};
}

native_stream_read_result
io_detail::wsa_socket_stream::_do_read_some(mutable_buffer mbuf) noexcept {
    ::WSABUF buf{.len = static_cast<ULONG>(mbuf.size()),
                 .buf = reinterpret_cast<CHAR*>(mbuf.data())};

    DWORD nsent = 0;
    DWORD flags = 0;
    ::SetLastError(0);
    ::WSARecv(native_handle(), &buf, 1, &nsent, &flags, nullptr, nullptr);
    return {{.bytes_transferred = nsent, .errn = ::WSAGetLastError()}};
}

#endif
