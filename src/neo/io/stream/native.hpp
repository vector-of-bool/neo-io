#pragma once

#ifdef _WIN32
#include <neo/io/stream/win32.hpp>
#else
#include <neo/io/stream/posix.hpp>
#endif

namespace neo {

using native_stream_handle = native_stream::native_handle_type;

using native_read_result  = read_result_t<native_stream>;
using native_write_result = write_result_t<native_stream>;

}  // namespace neo
