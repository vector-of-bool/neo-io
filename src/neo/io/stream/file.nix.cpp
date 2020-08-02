#if !_WIN32

#include <neo/io/stream/file.hpp>

#include <fcntl.h>
#include <unistd.h>

std::optional<neo::file_stream> neo::file_stream::open(const std::filesystem::path& fpath,
                                                       neo::open_mode               flags,
                                                       std::error_code&             ec) noexcept {
    int  open_mode = 0;
    auto flag_set  = [&](auto f) { return (int)flags & (int)f; };

    if (flag_set(open_mode::append)) {
        open_mode |= O_APPEND;
    }

    if (flag_set(open_mode::write)) {
        // If we are writing, and the user hasn't specified whether to create or not, default to
        // create
        if (!flag_set(open_mode::create) && !flag_set(open_mode::no_create)) {
            open_mode |= O_CREAT;
        }
        // Default to truncate the file
        if (!flag_set(open_mode::no_trunc)) {
            open_mode |= O_TRUNC;
        }
        if (flag_set(open_mode::read)) {
            open_mode |= O_RDWR;
        } else {
            open_mode |= O_WRONLY;
        }
    } else if (flag_set(open_mode::read)) {
        open_mode |= O_RDONLY;
    } else {
        // Neither read nor write was specified. Default to read.
        open_mode |= O_RDONLY;
    }

    if (flag_set(open_mode::create)) {
        open_mode |= O_CREAT;
    }

    if (flag_set(open_mode::trunc)) {
        open_mode |= O_TRUNC;
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
