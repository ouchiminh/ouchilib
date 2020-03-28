#pragma once
#include <utility>
#include <ratio>
#include <cstdint>

namespace ouchi::units {

template<class Tag, int Ex, class Ratio = std::ratio<1, 1>>
struct basic_dimension;

template<class Tag, int Ex, std::intmax_t Num, std::intmax_t Denom>
struct basic_dimension<Tag, Ex, std::ratio<Num, Denom>>{
    using ratio = typename std::ratio<Num, Denom>::type;
    using dimension = Tag;
    inline static constexpr int exponent = Ex;
    template<std::intmax_t N, std::intmax_t D>
    friend constexpr auto operator*(std::ratio<N, D>, basic_dimension)
        ->basic_dimension<Tag, Ex, typename std::ratio<Num * N, Denom * D>::type>
    { return {}; }
};

template<class Tag, class Ratio = std::ratio<1, 1>>
constexpr basic_dimension<Tag, 0, Ratio> make_dimension() { return {}; }

}

