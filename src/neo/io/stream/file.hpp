#pragma once

#include <neo/io/stream/native.hpp>

#include <filesystem>
#include <optional>
#include <system_error>

namespace neo {

enum class open_mode {
    read = 1,
    write,
    append,

    create,
    no_create,

    no_trunc,
    trunc,
};

class file_stream {
    native_stream _strm;

public:
    file_stream() = default;

    auto& native() noexcept { return _strm; }
    auto& native() const noexcept { return _strm; }

    file_stream(const std::filesystem::path& fpath)
        : file_stream(fpath, open_mode{}) {}

    file_stream(const std::filesystem::path& fpath, open_mode flags) {
        std::error_code ec;
        auto            fs = open(fpath, flags, ec);
        if (ec) {
            throw std::system_error(ec, "Failed to open file [" + fpath.string() + "]");
        }
        *this = std::move(*fs);
    }

    static file_stream open(const std::filesystem::path& fpath, open_mode flags) {
        return file_stream(fpath, flags);
    }

    static std::optional<file_stream>
    open(const std::filesystem::path& fpath, open_mode, std::error_code&) noexcept;

    template <buffer_range Bufs>
    auto write_some(const Bufs& b) noexcept {
        return _strm.write_some(b);
    }

    template <mutable_buffer_range Bufs>
    auto read_some(const Bufs& b) noexcept {
        return _strm.read_some(b);
    }
};

static_assert(read_write_stream<file_stream>);

}  // namespace neo
