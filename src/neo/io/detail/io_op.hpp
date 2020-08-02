#pragma once

#include <neo/io/completion_condition.hpp>
#include <neo/io/concepts/write_stream.hpp>

#include <neo/buffer_range_consumer.hpp>
#include <neo/const_buffer.hpp>

namespace neo {
namespace detail {

/**
 * Execute some I/O operation `op` until `cond` returns zero`, or `bufs` is
 * drained.
 */
template <typename Bufs, typename Cond, typename Op, bool UseBufferRanges>
constexpr auto
do_io_op(Bufs&& bufs_, Cond&& cond, std::bool_constant<UseBufferRanges>, Op&& op) noexcept {
    // consumer keeps track of advancing through the buffer as we read/write.
    buffer_range_consumer bufs{bufs_};

    using ResultType = decltype(op(bufs.prepare(1)));
    ResultType res_acc;
    res_acc.bytes_transferred = 0;

    while (true) {
        if (bufs.empty()) {
            // No more room in the output
            break;
        }
        auto tr_before = res_acc.bytes_transferred;
        // Check our completion condition
        const std::size_t req_to_transfer = std::invoke(cond, std::as_const(res_acc));
        if (req_to_transfer == 0) {
            // The condition wants to stop
            break;
        }
        if constexpr (UseBufferRanges) {
            // Don't attempt to transfer more than 10MB at once
            const std::size_t n_to_transfer
                = (std::min)(std::size_t(1024 * 1024 * 10), req_to_transfer);
            // Do a single operation with multiple buffers
            res_acc += op(bufs.prepare(n_to_transfer));
        } else {
            // The calling Op does not want buffer ranges (is not vectorized),
            // so just pass it the next contiguous buffer. Clamp the buffer size
            // to the size requested by the condition function.
            mutable_buffer next_buf = as_buffer(bufs.next_contiguous(), req_to_transfer);
            // Run the single op:
            res_acc += op(next_buf);
        }
        // Advance the buffer consumer by how much we transfered:
        auto n_transferred = res_acc.bytes_transferred - tr_before;
        bufs.consume(n_transferred);
        // If we had a failure, return now
        if (res_acc.is_failure()) {
            break;
        }
        // If no bytes were transferred, there's nothing we can do.
        if (n_transferred == 0) {
            break;
        }
    }

    return res_acc;
}

}  // namespace detail
}  // namespace neo