#pragma once

#include <neo/io/completion_condition.hpp>
#include <neo/io/concepts/write_stream.hpp>

#include <neo/buffers_consumer.hpp>
#include <neo/const_buffer.hpp>

namespace neo {
namespace detail {

/**
 * Execute some I/O operation `op` until `cond` returns zero`, or `bufs_io` is
 * drained.
 */
template <typename Bufs, typename Cond, typename Op, bool UseBufferRanges>
constexpr auto
do_io_op(Bufs&& bufs_, Cond&& cond, std::bool_constant<UseBufferRanges>, Op&& op) noexcept {
    // consumer keeps track of advancing through the buffer as we read/write.
    using consumer_type
        = std::conditional_t<UseBufferRanges, buffers_consumer<Bufs&>, buffers_vec_consumer<Bufs&>>;
    consumer_type bufs_io{bufs_};

    using ResultType = decltype(op(bufs_io.next(1)));
    ResultType res_acc;
    res_acc.bytes_transferred = 0;

    while (true) {
        auto tr_before = res_acc.bytes_transferred;
        // Check our completion condition
        const std::size_t req_to_transfer = std::invoke(cond, std::as_const(res_acc));
        if (req_to_transfer == 0) {
            // The condition wants to stop
            break;
        }
        auto next_out = bufs_io.next(req_to_transfer);
        if (!buffer_is_empty(next_out)) {
            res_acc = sum_transfer_result(res_acc, op(next_out));
        }
        // Advance the buffer consumer by how much we transfered:
        auto n_transferred = res_acc.bytes_transferred - tr_before;
        bufs_io.consume(n_transferred);
        // If we had a failure, return now
        if (transfer_errant(res_acc)) {
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