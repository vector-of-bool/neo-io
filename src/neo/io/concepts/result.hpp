#pragma once

#include <neo/concepts.hpp>

#include <cstdlib>

namespace neo {

// clang-format off
/**
 * A `transfer_result` is a type that represents the result of a transfer (read/write)
 * operation.
 */
template <typename T>
concept transfer_result =
    semiregular<T> &&
    requires(const T result, std::remove_cvref_t<T> mut_result) {
        { result.bytes_transferred } -> alike<std::size_t>;
        { result.is_failure() } noexcept -> alike<bool>;
        // XXX: Failes with GCC 9
        // { mut_result += result } noexcept -> same_as<std::remove_cvref_t<T>&>;
        { mut_result += result } noexcept;
    };
// clang-format on

/**
 * A prototype of `transfer_result`
 */
struct proto_transfer_result {
    std::size_t bytes_transferred = 0;
    bool        is_failure() const noexcept;

    proto_transfer_result operator+=(const proto_transfer_result&) noexcept;
};

}  // namespace neo
