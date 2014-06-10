#include "pch.hpp"
#include "generate.hpp"

using namespace yama;

//==============================================================================
rect_t
generate::rect(
    random_t&       random
  , rect_t    const bounds
  , int       const min_w
  , int       const min_h
) {
    BK_ASSERT(bounds);

    BK_ASSERT(min_w >= 0);
    BK_ASSERT(min_h >= 0);

    auto const w = bounds.width();
    auto const h = bounds.height();

    BK_ASSERT(w >= min_w);
    BK_ASSERT(h >= min_h);

    auto const leeway_x = (w - min_w) / 2;
    auto const leeway_y = (h - min_h) / 2;

    auto const left   = bounds.left   + random_uniform(random, 0, leeway_x);
    auto const top    = bounds.top    + random_uniform(random, 0, leeway_y);
    auto const right  = bounds.right  - random_uniform(random, 0, leeway_x);
    auto const bottom = bounds.bottom - random_uniform(random, 0, leeway_y);

    return {left, top, right, bottom};
}
//==============================================================================
point_t generate::point(
    random_t&       random
  , rect_t    const bounds
) {
    return {
        random_uniform(random, bounds.left, bounds.right - 1)
      , random_uniform(random, bounds.top, bounds.bottom - 1)
    };
}
//==============================================================================
generate::split_type
generate::get_split_type(
    random_t&       random
  , rect_t    const rect
  , int       const min_w
  , int       const min_h
  , float     const threshold
) {
    BK_ASSERT(min_w >= 0);
    BK_ASSERT(min_h >= 0);
    BK_ASSERT(threshold >= 1.0f);

    auto const w = rect.width();
    auto const h = rect.height();

    auto const min_split_w = min_w * 2;
    auto const min_split_h = min_h * 2;

    auto const can_split_v = w >= min_split_w;
    auto const can_split_h = h >= min_split_h;

    if (!can_split_v && !can_split_h) {
        return split_type::none;
    } else if ( can_split_v && !can_split_h) {
        return split_type::vertical;
    } else if (!can_split_v &&  can_split_h) {
        return split_type::horizontal;
    }
    ////else either split is possible

    bool const is_wider = w >= h;
    auto const float_w  = static_cast<float>(w);
    auto const float_h  = static_cast<float>(h);
    auto const ratio    = is_wider ? float_w / float_h : float_h / float_w;

    //threshhold exceeded?
    if (ratio > threshold) {
        if (is_wider) { return split_type::vertical; }
        else { return split_type::horizontal; }
    }

    //choose randomly
    return random_bool(random)
      ? split_type::vertical
      : split_type::horizontal;
}
//==============================================================================
generate::split_result
generate::split_rect(
    random_t&  random
  , rect_t     const rect
  , split_type       type
  , int        const min_w
  , int        const min_h
) {
    BK_ASSERT(min_w >= 0);
    BK_ASSERT(min_h >= 0);

    auto const w = rect.width();
    auto const h = rect.height();

    auto const min_split_w = min_w * 2;
    auto const min_split_h = min_h * 2;

    rect_t first  = rect;
    rect_t second = rect;

    if (type == split_type::vertical) {
        if (w < min_split_w) {
            return std::make_tuple(split_type::none, first, second);
        }

        auto const lo = rect.left + min_w;
        auto const hi = rect.right - min_w;
        auto const split = random_uniform(random, lo, hi);
        first.right = split;
        second.left = split;
    } else if (type == split_type::horizontal) {
        if (h < min_split_h) {
            return std::make_tuple(split_type::none, first, second);
        }

        auto const lo = rect.top + min_h;
        auto const hi = rect.bottom - min_h;
        auto const split = random_uniform(random, lo, hi);
        first.bottom = split;
        second.top = split;
    }

    return std::make_tuple(type, first, second);
}
//==============================================================================
