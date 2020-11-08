#include "./socket.hpp"

#include <cstring>
#include <ostream>

#if NEO_OS_IS_UNIX_LIKE
#include <netdb.h>
#include <sys/socket.h>
#elif NEO_OS_IS_WINDOWS
#include <WS2tcpip.h>
#endif

using namespace neo;

namespace {

class getaddrinfo_category_t : public std::error_category {
    const char* name() const noexcept override { return "getaddrinfo"; }

    std::string message(int rc) const {
        auto e_str = ::gai_strerror(rc);
        if (e_str) {
            return e_str;
        }
        return std::system_category().message(rc);
    }
};

}  // namespace

const std::error_category& neo::getaddrinfo_category() noexcept {
    static getaddrinfo_category_t inst;
    return inst;
}

address::family address::get_family() const noexcept {
    auto saddr = reinterpret_cast<const ::sockaddr_storage*>(_storage.data());
    switch (saddr->ss_family) {
    case AF_INET:
        return family::inet;
    case AF_INET6:
        return family::inet6;
    case AF_UNIX:
        return family::unixdom;
    case AF_UNSPEC:
        return family::unspecified;
    default:
        return family::unknown;
    }
}

std::optional<address> address::resolve(const std::string& host,
                                        const std::string& service,
                                        std::error_code&   ec) noexcept {
    io_detail::init_sockets();
    ::addrinfo* res = nullptr;
    auto        rc  = ::getaddrinfo(host.data(), service.data(), nullptr, &res);
    if (rc != 0) {
        ec = std::error_code(rc, getaddrinfo_category());
        return std::nullopt;
    }
    address ret;
    static_assert(sizeof(ret._storage) >= sizeof(sockaddr_storage));
    neo_assert(invariant,
               res->ai_addrlen <= sizeof(ret._storage),
               "Resolved address has a size greater than we can store. Huh?",
               res->ai_addrlen,
               sizeof(ret._storage),
               host,
               service,
               res->ai_family,
               res->ai_protocol,
               res->ai_socktype);
    std::memcpy(&ret._storage, res->ai_addr, res->ai_addrlen);
    ret._size = res->ai_addrlen;
    ::freeaddrinfo(res);
    return ret;
}

socket::~socket() {
#if NEO_OS_IS_UNIX_LIKE
    ::shutdown(native().native_handle(), SHUT_RDWR);
#elif NEO_OS_IS_WINDOWS
    // On windows, the underlying wsa_socket_stream will shutdown for us
#else
#error "We don't know how to shutdown sockets on this platform"
#endif
}

std::optional<neo::socket>
socket::create(address::family fam, socket::type typ, std::error_code& ec) noexcept {
    io_detail::init_sockets();
    auto af_fam = [&] {
        switch (fam) {
        case address::family::inet:
            return AF_INET;
        case address::family::inet6:
            return AF_INET6;
        case address::family::unixdom:
            return AF_UNIX;
        case address::family::unspecified:
            return AF_UNSPEC;
        case address::family::unknown:
            (void)0;
        }
        neo_assert(expects, false, "Invalid socket family given for socket::create()", int(fam));
    }();
    auto sock_typ = [&] {
        switch (typ) {
        case type::stream:
            return SOCK_STREAM;
        case type::datagram:
            return SOCK_DGRAM;
        }
        neo_assert(expects, false, "Invalid socket type given for socket::create()", int(typ));
    }();

    auto sockfd = ::socket(af_fam, sock_typ, 0);
    if (sockfd == -1) {
        ec = std::error_code(errno, std::system_category());
        return std::nullopt;
    }
    socket s;
    s.native().reset(std::move(sockfd));
    return s;
}

void socket::connect(address addr, std::error_code& ec) noexcept {
    io_detail::init_sockets();
    auto rc = ::connect(_stream.native_handle(),
                        reinterpret_cast<const ::sockaddr*>(&addr._storage),
                        static_cast<::socklen_t>(addr._size));
    if (rc) {
        ec = std::error_code(errno, std::system_category());
    }
}
