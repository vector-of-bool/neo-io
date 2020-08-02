#pragma once

#include <neo/io/stream/dynbuf.hpp>

#include <neo/as_dynamic_buffer.hpp>

#include <string>

namespace neo {

template <typename String>
class basic_string_stream {
public:
    String string;

    constexpr basic_string_stream() = default;
    constexpr explicit basic_string_stream(String&& s)
        : string(std::move(s)) {}
    constexpr explicit basic_string_stream(const String& s)
        : string(s) {}

    template <buffer_range Bufs>
    auto write_some(const Bufs& bufs) noexcept {
        return dynamic_buffer_stream(as_dynamic_buffer(string)).write_some(bufs);
    }

    template <mutable_buffer_range Bufs>
    auto read_some(const Bufs& bufs) noexcept {
        return dynamic_buffer_stream(as_dynamic_buffer(string)).read_some(bufs);
    }
};

using string_stream = basic_string_stream<std::string>;

}  // namespace neo