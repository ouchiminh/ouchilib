#pragma once
#include <utility>
#include <ratio>
#include <cstdint>

namespace ouchi::units {

template<class Tag, int Ex>
struct basic_dimension {
    using dimension = Tag;
    inline static constexpr int exponent = Ex;
};

template<class Tag>
constexpr basic_dimension<Tag, 0> make_dimension() { return {}; }

}

