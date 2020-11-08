#include "./layered.hpp"

#include <catch2/catch.hpp>

struct thing {};

template <typename Inner>
struct wrapper {
    using next_layer_type = Inner;
    Inner thing;

    const Inner& next_layer() const noexcept { return thing; }
};

TEST_CASE("Create a simple layered object") {
    thing          t;
    wrapper<thing> wrap{t};

    auto what = neo::next_layer(wrap);
    static_assert(std::is_same_v<decltype(what), thing>);

    auto bottom = neo::lowest_layer(wrap);
    static_assert(std::is_same_v<decltype(bottom), thing>);
}