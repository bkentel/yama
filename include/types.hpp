#pragma once

#include <cstdint>
#include <string>
#include <random>

#include "math.hpp"

namespace yama {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using utf8str = std::string;

using random_t = std::mt19937;

using rect_t          = yama::axis_aligned_rect<int>;
using point_t         = yama::point2d<int>;
using grid_position_t = yama::point2d<int>;
using line_t          = yama::line2d<int>;

} //namespace yama
