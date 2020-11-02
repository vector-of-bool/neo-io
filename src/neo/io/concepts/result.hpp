#pragma once

#include <neo/concepts.hpp>

#include <cstdlib>
#include <string_view>
#include <system_error>

namespace neo {

// clang-format off

namespace io_detail {

template <typename T>
concept has_has_error = requires(const T r) {
    { r.has_error() } noexcept -> simple_boolean;
};

}  // namespace io_detail

/**
 * A `transfer_result` is a type that represents the result of a transfer (read/write)
 * operation.
 */
template <typename T>
concept transfer_result =
    semiregular<std::remove_cvref_t<T>> &&
    requires(const T result, std::remove_cvref_t<T> mut_result, std::size_t size) {
        { result.bytes_transferred } -> alike<std::size_t>;
        { mut_result.bytes_transferred += size } -> alike<std::size_t>;
        { result.error() } -> convertible_to<std::error_code>;
        { result.error() } -> simple_boolean;
    };
// clang-format on

/**
 * A prototype of `transfer_result`
 */
struct proto_transfer_result {
    std::size_t     bytes_transferred = 0;
    std::error_code error() const noexcept;
};

struct basic_transfer_result {
    std::size_t     bytes_transferred = 0;
    std::error_code ec{};

    [[nodiscard]] auto error() const noexcept { return ec; }
};

/**
 * @brief Determine whether a given transfer result encountered an error
 *
 * @param r The transfer_result to check
 * @return true If the .error() of the transfer is 'truthy'
 * @return false Otherwise
 *
 * @note This also checks for an `.has_error()` method to check more easily if
 * the type contains an error, without requiring the creation of a std::error_code
 */
template <transfer_result Res>
[[nodiscard]] constexpr bool transfer_errant(Res&& r) noexcept {
    if constexpr (io_detail::has_has_error<Res>) {
        return r.has_error();
    } else {
        return !!r.error();
    }
}

/**
 * @brief If the given transfer result has an error, throws an instance of
 * `std::system_error`.
 *
 * Constructs the exception as 'system_error(r.error(), string(msg))`.
 *
 * @param r The transfer result to check
 * @param msg A message to attach to a (potential) system_error
 */
template <transfer_result Res>
constexpr void throw_if_transfer_errant(Res&& r, std::string_view msg) {
    if (transfer_errant(r)) {
        throw std::system_error(r.error(), std::string(msg));
    }
}

/**
 * @brief "Accumulate" two transfer results.
 *
 * The `.bytes_transferred` value will be sum of those of the two parameters.
 * All other state is copied from the right-hand parameter. This preserves
 * the error-information of the right-hand parameter, so that if it represents
 * an error, the summed result will also have that error.
 *
 * @param left The "left hand" addend. Should be the "previous" result.
 * @param right The "right hand" addend. Should be the "new" result.
 * @return A result with .bytes_transferred being the sum, but all other
 *  information copied from the right.
 */
template <transfer_result Res, alike<Res> Other>
[[nodiscard]] constexpr Other sum_transfer_result(Res&& left, Other right) noexcept {
    right.bytes_transferred += left.bytes_transferred;
    return right;
}

}  // namespace neo
