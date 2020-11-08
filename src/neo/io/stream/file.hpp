#pragma once

#include <neo/io/stream/native.hpp>

#include <neo/assert.hpp>
#include <neo/buffer_range.hpp>
#include <neo/enum.hpp>
#include <neo/error.hpp>

#include <filesystem>
#include <optional>
#include <system_error>

namespace neo {

/**
 * @brief Options to control file-open behavior.
 */
enum class open_mode : int {
    /// No options set. Implies `read`
    none = 0,

    /// Open the file for reading
    read = 1,
    /// Open the file for writing
    write = 1 << 1,
    /// Open the file for reading and writing, and do not truncate
    append = 1 << 2,

    /// Create the file if it does not exist. Default for 'write' and 'append'
    create = 1 << 3,
    /// Create the file, unless it already exists, in which case emit an error
    create_exclusive = 1 << 4,
    /// Only open an existing file, never creating one implicitly. Default for 'write'
    no_create = 1 << 5,

    /// If opening an existing file, leave its content available. Default for 'append' and 'read'
    no_trunc = 1 << 6,
    /// If opening an existing file, truncate the file's content. Default for 'write'
    trunc = 1 << 7,
};

NEO_DECL_ENUM_BITOPS(open_mode);

/**
 * @brief Fill out all flags implied by the given open_mode flags.
 *
 * @param flags A set of open_mode flags
 * @return open_mode New flags, with all implied flags set
 */
constexpr open_mode default_open_flags(open_mode flags = open_mode()) noexcept {
    using om = open_mode;

    // Default open mode is to read
    if (flags == om::none) {
        flags = om::read;
    }

    auto is_set = test_flags(&flags);

    // If appending, we want to both read and write
    if (is_set(om::append)) {
        flags |= om::read;
        flags |= om::write;
    }

    // create_exclusive implies create
    if (is_set(om::create_exclusive)) {
        flags |= om::create;
    }

    if (is_set(om::write)) {
        // If not told otherwise, writing will create the file if it does not exist
        if (!is_set(om::no_create)) {
            flags |= om::create;
        }
        // By default, opening a file to write will truncate it (except if appending)
        if (!is_set(om::append) && !is_set(om::no_trunc)) {
            flags |= om::trunc;
        }
    }

    // If create is not set by now, do not no_create
    if (!is_set(om::create)) {
        flags |= om::no_create;
    }

    // If trunc is not set by now, do not truncate
    if (!is_set(om::trunc)) {
        flags |= om::no_trunc;
    }

    neo_assert(expects,
               is_set(om::trunc) != is_set(om::no_trunc),
               "Conflicting options open_mode::trunc and open_mode::no_trunc specified",
               is_set(om::read),
               is_set(om::write),
               is_set(om::append),
               is_set(om::create),
               is_set(om::no_create),
               is_set(om::create_exclusive),
               is_set(om::trunc),
               is_set(om::no_trunc));

    neo_assert(expects,
               is_set(om::create) != is_set(om::no_create),
               "Conflicting options open_mode::create and open_mode::no_create specified",
               is_set(om::read),
               is_set(om::write),
               is_set(om::append),
               is_set(om::create),
               is_set(om::no_create),
               is_set(om::create_exclusive),
               is_set(om::trunc),
               is_set(om::no_trunc));

    return flags;
}

/**
 * @brief A stream corresponding to a file.
 */
class file_stream {
    native_stream _strm;

public:
    /// Default-construct an invalid file stream
    file_stream() = default;

    /// Access the underlying native stream
    auto& native() noexcept { return _strm; }
    auto& native() const noexcept { return _strm; }

    void close() noexcept { _strm.close(); }

    /**
     * @brief Open a file for reading, given by 'fpath'
     *
     * @param fpath Path to an existing file to open for reading
     */
    file_stream(const std::filesystem::path& fpath)
        : file_stream(fpath, open_mode{}) {}

    /**
     * @brief Open a file at the given path, using the open mode flags
     *
     * @param fpath The path to a file
     * @param flags The flags to control the behavior of the open operation
     */
    file_stream(const std::filesystem::path& fpath, open_mode flags) {
        error_code_thrower err;
        auto               fs = open(fpath, flags, err);
        err("Failed to open file [{}]", fpath.string());
        *this = std::move(*fs);
    }

    /**
     * @brief Return an opened file stream for the file at the given path.
     *
     * The returned file stream will always be open and valid. If an error
     * occurs, this function will throw an exception.
     */
    static file_stream open(const std::filesystem::path& fpath, open_mode flags) {
        return file_stream(fpath, flags);
    }

    /**
     * @brief Returne a valid file stream opened for reading.
     */
    static file_stream open(const std::filesystem::path& fpath) {
        return open(fpath, open_mode::read);
    }

    /**
     * @brief Return an opened file, or a nullopt in case of error.
     *
     * @param ec An error code that will receive the error if the open operation fails
     */
    static std::optional<file_stream>
    open(const std::filesystem::path& fpath, open_mode, std::error_code& ec) noexcept;

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
