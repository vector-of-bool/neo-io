#pragma once

#include <neo/io/completion_condition.hpp>
#include <neo/io/concepts/read_stream.hpp>

#include <neo/io/detail/io_op.hpp>

#include <neo/buffer_range_consumer.hpp>
#include <neo/const_buffer.hpp>

namespace neo {

/**
 * Read from the given stream into the buffers until a completion condition is met.
 *
 * @param strm The stream to read form
 * @param bufs The buffers that should receive the data
 * @param cond The completion condition. Should return the number of bytes to be read from the
 *      stream.
 */
template <read_stream                           Stream,
          mutable_buffer_range                  Bufs,
          read_completion_condition_for<Stream> CompletionCondition>
read_result_t<Stream, Bufs> read(Stream&& strm, Bufs&& bufs, CompletionCondition&& cond) noexcept {
    auto use_buf_ranges = std::bool_constant<vectored_read_stream<Stream>>{};
    return detail::do_io_op(bufs, cond, use_buf_ranges, [&](auto&& parts) {
        return strm.read_some(parts);
    });
}

template <read_stream Stream, mutable_buffer_range Bufs>
read_result_t<Stream> read(Stream& strm, Bufs&& bufs) noexcept {
    return read(strm, bufs, transfer_all);
}

}  // namespace neo
