#pragma once

#include <neo/io/completion_condition.hpp>
#include <neo/io/concepts/write_stream.hpp>

#include <neo/io/detail/io_op.hpp>

#include <neo/buffer_range_consumer.hpp>
#include <neo/const_buffer.hpp>

namespace neo {

/**
 * Write data from the given buffers into the given stream until the completion
 * condition returns zero or we exhaust the buffer sequence.
 *
 * @arg `strm` The stream that we write into
 * @arg `bufs` The buffer (sequence) that we will write into the stream.
 * @arg `cond` A completion condition that aides in transferring data.
 *
 * Before each `write_some` operation, the `cond` object will be invoked with
 * the partial write result of the ongoing operation. The `cond` must return
 * a std::size_t which represents the maximum size of the next write operation.
 * The `bytes_transferred` of the partial result will be the total number of
 * bytes that have been read so far during the entire operation.
 *
 * If the size returned by the condition is larger than the unwritten portion
 * of the buffer, the remaining number of bytes will be written instead. (i.e.
 * it is safe to return arbitrarily large values from the completion
 * condition.)
 */
template <write_stream                           Stream,
          buffer_range                           Bufs,
          write_completion_condition_for<Stream> CompletionCondition>
write_result_t<Stream> write(Stream& strm, const Bufs& bufs, CompletionCondition&& cond) noexcept {
    auto use_buf_ranges = std::bool_constant<vectored_write_stream<Stream>>{};
    return detail::do_io_op(bufs, cond, use_buf_ranges, [&](auto&& bufs) {
        return strm.write_some(bufs);
    });
}

/**
 * Write the entire contents of the given buffer into the given stream.
 *
 * Equivalent to: write(strm, b, transfer_all)
 */
template <write_stream Stream, buffer_range Bufs>
write_result_t<Stream> write(Stream& strm, const Bufs& b) noexcept {
    return write(strm, b, transfer_all);
}

}  // namespace neo
