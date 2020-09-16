#pragma once

#include <neo/platform.hpp>

#include <neo/io/stream/native.hpp>

#include <optional>
#include <system_error>

namespace neo {

namespace io_detail {

#if NEO_OS_IS_WINDOWS
void init_sockets() noexcept;
#else
inline void init_sockets() noexcept {}
#endif

struct wsabuf {
    std::uint32_t len;
    char*         buf;
};

class wsa_socket_stream {
public:
    using native_handle_type = std::uint64_t;

    constexpr static native_handle_type invalid_native_handle_value = ~native_handle_type(0);

private:
    static thread_local std::size_t _tl_wsabufs_arr_len;
    static thread_local wsabuf*     _tl_small_wsabufs_array;

    std::uint64_t _native_handle = invalid_native_handle_value;

    native_stream_write_result _do_write_arr(std::size_t nbufs) noexcept;
    native_stream_read_result  _do_read_arr(std::size_t nbufs) noexcept;

    native_stream_write_result _do_write_some(const_buffer buf) noexcept;
    native_stream_write_result _do_write_some(mutable_buffer buf) noexcept {
        return _do_write_some(buf);
    }

    native_stream_read_result _do_read_some(mutable_buffer) noexcept;

    template <buffer_range T>
    std::size_t _prep_wsabufs_arr(T&& bufs) noexcept {
        using std::begin;
        using std::end;
        std::size_t n_bufs   = 0;
        auto        buf_it   = begin(bufs);
        auto        buf_stop = end(bufs);
        auto        out_it   = _tl_small_wsabufs_array;
        auto        out_end  = _tl_small_wsabufs_array + _tl_wsabufs_arr_len;
        for (; buf_it != buf_stop && out_it != out_end; ++buf_it, ++out_it) {
            auto buf    = *buf_it;
            out_it->buf = reinterpret_cast<char*>(const_cast<std::byte*>(buf.data()));
            out_it->len = static_cast<std::uint32_t>(buf.size());
        }
        return n_bufs;
    }

    template <mutable_buffer_range T>
    native_stream_read_result _do_read_some(T&& bufs) noexcept {
        auto n_bufs = _prep_wsabufs_arr(bufs);
        return _do_read_arr(n_bufs);
    }

    template <buffer_range T>
    native_stream_write_result _do_write_some(T&& bufs) noexcept {
        auto n_bufs = _prep_wsabufs_arr(bufs);
        return _do_write_arr(n_bufs);
    }

public:
    constexpr wsa_socket_stream() = default;

    wsa_socket_stream(wsa_socket_stream&& o) noexcept
        : _native_handle(std::exchange(o._native_handle, invalid_native_handle_value)) {}

    wsa_socket_stream& operator=(wsa_socket_stream&& o) noexcept {
        reset(std::exchange(o._native_handle, invalid_native_handle_value));
        return *this;
    }

    ~wsa_socket_stream() { shutdown(); }
    void shutdown();

    constexpr native_handle_type native_handle() const noexcept { return _native_handle; }

    void reset(native_handle_type&& sock) noexcept {
        shutdown();
        _native_handle = sock;
    }

    template <mutable_buffer_range Bufs>
    [[nodiscard]] native_stream_read_result read_some(Bufs out) noexcept {
        return _do_read_some(out);
    }

    template <buffer_range Bufs>
    [[nodiscard]] native_stream_write_result write_some(Bufs in) noexcept {
        return _do_write_some(in);
    }
};

#if NEO_OS_IS_WINDOWS
using native_socket_stream = wsa_socket_stream;
#else
using native_socket_stream = native_stream;
#endif

}  // namespace io_detail

const std::error_category& getaddrinfo_category() noexcept;

class address {
    friend class socket;
    std::array<std::byte, 128> _storage{};

public:
    enum class family {
        unknown,
        unspecified,
        inet,
        inet6,
        unixdom,
    };

    address() = default;

    family get_family() const noexcept;

    static std::optional<address>
    resolve(const std::string& host, const std::string& service, std::error_code&) noexcept;

    static address resolve(const std::string& host, const std::string& service) {
        std::error_code ec;
        auto            addr = resolve(host, service, ec);
        if (ec) {
            throw std::system_error(ec,
                                    "Failed to resolve host " + std::string(host)
                                        + " with service/port " + std::string(service));
        }
        return *addr;
    }
};

class socket {
    io_detail::native_socket_stream _stream;

public:
    enum class type {
        stream,
        datagram,
    };

    socket()         = default;
    socket(socket&&) = default;
    ~socket();

    auto& native() noexcept { return _stream; }
    auto& native() const noexcept { return _stream; }

    static std::optional<socket> create(address::family, type, std::error_code& ec) noexcept;
    static socket                create(address::family fam, type ty) {
        std::error_code ec;
        auto            s = create(fam, ty, ec);
        if (ec) {
            throw std::system_error(ec, "Failed to create a new socket");
        }
        return std::move(*s);
    }

    static std::optional<socket>
    open_connected(address addr, type typ, std::error_code& ec) noexcept {
        auto s = create(addr.get_family(), typ, ec);
        if (s) {
            s->connect(addr, ec);
        }
        return s;
    }

    static socket open_connected(address addr, type typ) {
        std::error_code ec;
        auto            sock = open_connected(addr, typ, ec);
        if (ec) {
            throw std::system_error(ec, "Failed to connect socket");
        }
        return std::move(*sock);
    }

    void connect(address addr, std::error_code& ec) noexcept;

    template <buffer_range Bufs>
    auto write_some(Bufs&& b) noexcept requires requires {
        _stream.write_some(b);
    }
    { return _stream.write_some(b); }

    template <mutable_buffer_range Bufs>
    auto read_some(Bufs&& b) noexcept requires requires {
        _stream.read_some(b);
    }
    { return _stream.read_some(b); }
};

}  // namespace neo
