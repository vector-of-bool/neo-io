#pragma once

#include <neo/concepts.hpp>
#include <neo/fwd.hpp>
#include <neo/ref.hpp>

namespace neo {

// clang-format off
template <typename T>
concept layered = requires(T item) {
    { item.next_layer() } noexcept;
};
// clang-format on

template <layered T>
decltype(auto) next_layer(T&& stream) noexcept {
    return stream.next_layer();
}

template <layered T>
using next_layer_t = std::remove_cvref_t<decltype(ref_v<T>.next_layer())>;

template <typename T>
decltype(auto) lowest_layer(T&& item) noexcept {
    return NEO_FWD(item);
}

template <layered T>
decltype(auto) lowest_layer(T&& stream) noexcept {
    return lowest_layer(next_layer(stream));
}

template <typename T>
using lowest_layer_t = std::remove_cvref_t<decltype(lowest_layer(std::declval<T&>()))>;

}  // namespace neo
