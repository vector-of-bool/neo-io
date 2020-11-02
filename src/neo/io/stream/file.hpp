#pragma once

#include <neo/io/stream/native.hpp>

#include <neo/buffer_range.hpp>
#include <neo/enum.hpp>

#include <filesystem>
#include <optional>
#include <system_error>

namespace neo {

enum class open_mode : int {
    none   = 0,
    read   = 1,
    write  = 1 << 1,
    append = 1 << 2,

    create           = 1 << 3,
    create_exclusive = 1 << 4,
    no_create        = 1 << 5,

    no_trunc = 1 << 6,
    trunc    = 1 << 7,
};

NEO_DECL_ENUM_BITOPS(open_mode);

constexpr open_mode default_open_flags(open_mode flags = open_mode()) noexcept {
    using om = open_mode;
    // Default open mode is to read
    if (flags == om::none) {
        flags = om::read;
    }

    // If appending, we want to both read and write
    if (test_flags(flags, om::append)) {
        flags |= om::read;
        flags |= om::write;
    }

    if (test_flags(flags, om::write)) {
        // If not told otherwise, writing will create the file if it does not exist
        if (!test_flags(flags, om::no_create)) {
            flags |= om::create;
        }
        // By default, opening a file to write will truncate it (except if appending)
        if (!test_flags(flags, om::append) && !test_flags(flags, om::no_trunc)) {
            flags |= om::trunc;
        }
    }

    return flags;
}

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
    auto write_some(Bufs&& b) noexcept requires requires {
        _strm.write_some(b);
    }
    { return _strm.write_some(b); }

    template <mutable_buffer_range Bufs>
    auto read_some(Bufs&& b) noexcept requires requires {
        _strm.read_some(b);
    }
    { return _strm.read_some(b); }
};

}  // namespace neo
