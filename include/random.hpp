#pragma once

#include "types.hpp"
#include "math.hpp"

////////////////////////////////////////////////////////////////////////////////
//! @file
//! Random number generation.
////////////////////////////////////////////////////////////////////////////////

namespace yama {

template <typename Random, typename T>
inline auto random_uniform(
    Random& random
  , closed_integral_interval<T> const range
) {
    return std::uniform_int_distribution<>{range.lower, range.upper}(random);
}

template <typename Random>
inline int random_uniform(Random& random, int const low, int const hi) {
    return std::uniform_int_distribution<>{low, hi}(random);
}

template <typename Random>
inline bool random_bool(Random& random) {
    return std::uniform_int_distribution<>{0, 1}(random) == 1;
}

////////////////////////////////////////////////////////////////////////////////
//! Roll a percentage [0, 100] -> 101 values
////////////////////////////////////////////////////////////////////////////////
template <typename T = int, typename Random>
inline int random_percent(Random& random) {
    return std::uniform_int_distribution<T>{0, 100}(random);
}

//template <typename Random>
//inline vector2d<float> random_unit_vector(Random& random) {
//    auto const dist = std::uniform_real_distribution<float> {-1.0f, 1.0f};
//    return vector2d<float> {dist(random), dist(random)}.norm();
//}

}
