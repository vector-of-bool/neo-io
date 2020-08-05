#include <neo/io/stream/file.hpp>

#if _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

std::optional<neo::file_stream> neo::file_stream::open(const std::filesystem::path& fpath,
                                                       neo::open_mode               open_flags,
                                                       std::error_code&             ec) noexcept {
    DWORD access     = 0;
    DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD creation   = 0;
    DWORD attrs      = FILE_ATTRIBUTE_NORMAL;
    DWORD flags      = 0;

    using om       = open_mode;
    auto test_flag = [&](om f) { return (int)open_flags & (int)f; };
    open_flags     = default_open_flags(open_flags);

    if (test_flag(om::write)) {
        access |= GENERIC_WRITE;
    }
    if (test_flag(om::read)) {
        access |= GENERIC_READ;
    }

    if (test_flag(om::create_exclusive)) {
        creation = CREATE_NEW;
    } else if (test_flag(om::create)) {
        if (test_flag(om::trunc)) {
            // Create a new file, overwriting a previous
            creation = CREATE_ALWAYS;
        } else {
            // Create a file if absent, or just open the existing one.
            creation = OPEN_ALWAYS;
        }
    } else {
        if (test_flag(om::trunc)) {
            // Open an existing file and truncate it.
            creation = TRUNCATE_EXISTING;
        } else {
            creation = OPEN_EXISTING;
        }
    }

    SECURITY_ATTRIBUTES security = {
        .nLength              = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = nullptr,
        .bInheritHandle       = true,
    };

    ::SetLastError(0);
    auto hndl = ::CreateFileW(fpath.native().c_str(),
                              access,
                              share_mode,
                              &security,
                              creation,
                              attrs | flags,
                              nullptr);
    if (hndl == INVALID_HANDLE_VALUE) {
        ec = std::error_code(::GetLastError(), std::system_category());
        return std::nullopt;
    }

    file_stream ret;
    ret._strm = native_stream::from_native_handle(std::move(hndl));

    if (test_flag(om::append)) {
        LONG high_offset = 0;
        ::SetLastError(0);
        ::SetFilePointer(ret.native().native_handle(), 0, &high_offset, FILE_END);
        neo_assert_always(invariant,
                          ::GetLastError() == 0,
                          "SetFilePointer failed to seek for file append. This should never occur?",
                          fpath.string(),
                          ::GetLastError(),
                          int(open_flags));
    }

    return std::move(ret);
}
#endif
