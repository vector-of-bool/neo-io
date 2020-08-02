#pragma once

#include <neo/io/concepts/stream.hpp>

#include <neo/concepts.hpp>

namespace neo {

// clang-format off
template <typename T, typename Stream>
concept write_completion_condition_for =
    neo::invocable<T, write_result_t<Stream>> &&
    neo::same_as<std::invoke_result_t<T, write_result_t<Stream>>, std::size_t>;

template <typename T, typename Stream>
concept read_completion_condition_for =
    neo::invocable<T, read_result_t<Stream>> &&
    neo::same_as<std::invoke_result_t<T, read_result_t<Stream>>, std::size_t>;
// clang-format on

/**
 * Transfer completion condition that transfers an entire buffer.
 */
inline constexpr struct transfer_all_t {
    template <transfer_result W>
    constexpr std::size_t operator()(const W&) const noexcept {
        return std::numeric_limits<std::size_t>::max();
    }
} transfer_all;

/**
 * Transfer completion condition that transfers *exaclty* the requested number
 * of bytes.
 */
struct transfer_exactly {
    std::size_t _size;

    template <transfer_result R>
    constexpr std::size_t operator()(const R& res) const noexcept {
        return _size - res.bytes_transferred;
    }
};

}  // namespace neo
