#pragma once

#include "types.hpp"
#include "math.hpp"

namespace yama {

template <typename Random, typename T>
inline T random_uniform(Random& random, T low, T hi) {
    return std::uniform_int_distribution<T>{low, hi}(random);
}

template <typename Random>
inline bool random_bool(Random& random) {
    return std::uniform_int_distribution<>{0, 1}(random) == 1;
}

////////////////////////////////////////////////////////////////////////////////
//! Roll a percentage [0, 100] -> 101 values
////////////////////////////////////////////////////////////////////////////////
template <typename Random>
inline typename Random::result_type roll_100(Random& random) {
    using result_t = typename Random::result_type;
    return std::uniform_int_distribution<result_t>{0, 100}(random);
}

//template <typename Random>
//inline vector2d<float> random_unit_vector(Random& random) {
//    auto const dist = std::uniform_real_distribution<float> {-1.0f, 1.0f};
//    return vector2d<float> {dist(random), dist(random)}.norm();
//}

}
