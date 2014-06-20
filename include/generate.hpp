////////////////////////////////////////////////////////////////////////////////
//! @file
//! Random (procedural) generation.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "types.hpp"
#include "math.hpp"
#include "random.hpp"

namespace yama {
namespace generate {

////////////////////////////////////////////////////////////////////////////////
//! @return A random integer in [lower, upper] weighted toward one side.
//!
//! @param random
//!     Random number generator.
//! @param range
//!     The closed range of values.
//! @param weight
//!    * For weight > 0 values are skewed toward range.upper.
//!    * For weight < 0 values are skewed toward range.lower.
//! @param variance
//!    * For variance > 0 values have more variance.
//!    * For variance < 0 values have less variance.
////////////////////////////////////////////////////////////////////////////////
int weighted_range(
    random_t&                     random
  , closed_integral_interval<int> range
  , closed_range<int, 100>        weight   = 0
  , closed_range<int, 100>        variance = 0
);

////////////////////////////////////////////////////////////////////////////////
//! @return A random rect contained within @p bounds.
//!
//! @param random
//!     Random number generator.
//! @param bounds
//!     Bounds that contain the resulting rect.
//! @param min_w
//!      Minimum width of the resulting rect.
//! @param min_h
//!     Minimum height of the resulting rect.
//! @param border_size
//!     The amount of empty space to leave in the upper left of bounds.
//!
//! @pre @p bounds is normalized.
////////////////////////////////////////////////////////////////////////////////
rect_t
uniform_bounded_rect(
    random_t&     random
  , rect_t        bounds
  , positive<int> min_w
  , positive<int> min_h
  , positive<int> border_size = 0
);

////////////////////////////////////////////////////////////////////////////////
//! @return A random rect contained within @p bounds.
//!
//! @param random
//!     Random number generator.
//! @param bounds
//!     Bounds that contain the resulting rect.
//! @param min_w
//!      Minimum width of the resulting rect.
//! @param min_h
//!     Minimum height of the resulting rect.
//! @param weight
//!    * For weight > 0 values are skewed toward range.upper.
//!    * For weight < 0 values are skewed toward range.lower.
//! @param variance
//!    * For variance > 0 values have more variance.
//!    * For variance < 0 values have less variance.
//!
//! @pre @p bounds is normalized.
////////////////////////////////////////////////////////////////////////////////
rect_t
weighted_bounded_rect(
    random_t&              random
  , rect_t                 bounds
  , positive<int>          min_w
  , positive<int>          min_h
  , closed_range<int, 100> weight      = 0
  , closed_range<int, 100> variance    = 0
  , positive<int>          border_size = 0
);

////////////////////////////////////////////////////////////////////////////////
//! @return A random point.
//!
//! @param random Random number generator.
//! @param bounds Bounds that contain the resulting point.
//!
//! @pre @p bounds is normalized.
////////////////////////////////////////////////////////////////////////////////
point_t
bounded_point(
    random_t& random
  , rect_t    bounds
);

////////////////////////////////////////////////////////////////////////////////
//! The split direction for split_rect.
////////////////////////////////////////////////////////////////////////////////
enum class split_type {
    none       //!< Can't be split.
  , vertical   //!< Split vertically.
  , horizontal //!< Split horizontally.
};

////////////////////////////////////////////////////////////////////////////////
//! The result of a rect split. TODO
////////////////////////////////////////////////////////////////////////////////
using split_result = std::tuple<split_type, rect_t, rect_t>;

////////////////////////////////////////////////////////////////////////////////
//! @return The direction a rect should be split (if possible).
//!
//! @param rect The rect to split.
//! @param min_w The minimum width the resulting rects.
//! @param min_h The minimum height of the resulting rects.
//! @param threshold The maximum ratio between the longest dimension and the
//! shortest dimension allowed before the rect must be split along the long dimension.
//!
//! @pre rect is normalized.
//! @pre threshold_num >= 1 //TODO
////////////////////////////////////////////////////////////////////////////////
split_type
get_split_type(
    random_t&     random
  , rect_t        rect
  , positive<int> min_w     = 0
  , positive<int> min_h     = 0
  , float         threshold = 1.0f
);

////////////////////////////////////////////////////////////////////////////////
//! @return @p rect split in the direction given by @p type,
//! or @p rect iff @p type is split_type::none
//!
//! @param rect The rect to split.
//! @param type The split direction.
//! @param min_w The minimum width the resulting rects.
//! @param min_h The minimum height of the resulting rects.
//!
//! @pre rect is normalized.
////////////////////////////////////////////////////////////////////////////////
split_result
split_rect(
    random_t&     random
  , rect_t        rect
  , split_type    type
  , positive<int> min_w = 0
  , positive<int> min_h = 0
);

} //namespace generate
} //namespace yama
