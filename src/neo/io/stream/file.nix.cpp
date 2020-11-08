#include <neo/io/stream/file.hpp>

#if !_WIN32

#include <fcntl.h>
#include <unistd.h>

std::optional<neo::file_stream> neo::file_stream::open(const std::filesystem::path& fpath,
                                                       neo::open_mode               open_flags,
                                                       std::error_code&             ec) noexcept {
    int open_mode = 0;
    using om      = neo::open_mode;

    neo_assertion_breadcrumbs("Opening file", fpath.string());

    open_flags  = default_open_flags(open_flags);
    auto is_set = test_flags(open_flags);

    if (is_set(om::append)) {
        open_mode |= O_APPEND;
    }

    if (is_set(om::read)) {
        if (is_set(om::write)) {
            open_mode = O_RDWR;
        } else {
            open_mode = O_RDONLY;
        }
    } else if (is_set(om::write)) {
        open_mode = O_WRONLY;
    }

    if (is_set(om::trunc)) {
        // Truncate the file upon opening
        open_mode |= O_TRUNC;
    }

    if (is_set(om::create_exclusive)) {
        open_mode |= O_CREAT | O_EXCL;
    } else if (is_set(om::create)) {
        open_mode |= O_CREAT;
    } else {
        // The file should already exist
    }

    open_mode |= O_CLOEXEC;

    int file_mode = 0b110'110'100;

    auto fd = ::open(fpath.string().c_str(), open_mode, file_mode);

    if (fd == -1) {
        ec = std::error_code(errno, std::system_category());
        return std::nullopt;
    }
    neo::file_stream ret;
    ret._strm = neo::native_stream::from_native_handle(std::move(fd));
    return ret;
}

#endif  // !_WIN32
