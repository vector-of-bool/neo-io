#include "./socket.hpp"

#if NEO_OS_IS_WINDOWS
namespace {

void wsa_initialize() {
    WSADATA wsd;
    ::WSAStartup(MAKEWORD(2, 2), &wsd);
}

}  // namespace
#endif