#include "pch.hpp"
#include "generate.hpp"

using namespace yama;

//==============================================================================
int generate::weighted_range(
    random_t&                           random
  , closed_integral_interval<int> const range
  , closed_range<int, 100>        const weight
  , closed_range<int, 100>        const variance
) {
    using T = double;

    constexpr auto range_mean   = T{decltype(weight)::check_type::range};
    constexpr auto range_stddev = T{decltype(variance)::check_type::range};

    auto const mean   = ((weight   / range_mean)   + T{1}) / T{2};
    auto const stddev = ((variance / range_stddev) + T{1}) / T{2};
    auto       dist   = std::normal_distribution<T> {mean, stddev};
    
    auto const a           = static_cast<T>(range.lower);
    auto const b           = static_cast<T>(range.upper);
    auto const delta       = b - a;
    auto const lower_limit = (T{0}  - T{0.5}) / delta;
    auto const upper_limit = (delta + T{0.5}) / delta;

    auto const in_range = [&](T const n) {
        return n > lower_limit && n < upper_limit;
    };

    auto n = dist(random);
    while (!in_range(n)) {
        n = dist(random);
    }

    return static_cast<int>(std::round(a + delta * n));
}
//==============================================================================
rect_t
generate::uniform_bounded_rect(
    random_t&           random
  , rect_t              bounds
  , positive<int> const w
  , positive<int> const h
  , positive<int> const border_size
) {
    bounds.left += border_size;
    bounds.top  += border_size;

    auto const max_w = bounds.width();
    auto const max_h = bounds.height();

    BK_ASSERT(w <= max_w);
    BK_ASSERT(h <= max_h);

    auto const leeway_x = max_w - w;
    auto const leeway_y = max_h - h;

    auto const dx = yama::random_uniform(random, 0, leeway_x);
    auto const dy = yama::random_uniform(random, 0, leeway_y);

    auto const left = bounds.left + dx;
    auto const top  = bounds.top  + dy;

    return {left, top, left + w, top + h};
}

rect_t
generate::weighted_bounded_rect(
    random_t&                    random
  , rect_t                       bounds
  , positive<int>          const min_w
  , positive<int>          const min_h
  , closed_range<int, 100> const weight
  , closed_range<int, 100> const variance
  , positive<int>          const border_size
) {
    bounds.left += border_size;
    bounds.top  += border_size;

    auto const max_w = bounds.width();
    auto const max_h = bounds.height();

    auto const w = weighted_range(random, make_closed_interval(min_w, max_w), weight, variance);
    auto const h = weighted_range(random, make_closed_interval(min_h, max_h), weight, variance);

    return uniform_bounded_rect(random, bounds, positive<int>{w}, positive<int>{h});
}
//==============================================================================
point_t
generate::bounded_point(
    random_t&       random
  , rect_t    const bounds
) {
    return {
        random_uniform(random, bounds.left, bounds.right  - 1)
      , random_uniform(random, bounds.top,  bounds.bottom - 1)
    };
}
//==============================================================================
generate::split_type
generate::get_split_type(
    random_t&           random
  , rect_t        const rect
  , positive<int> const min_w
  , positive<int> const min_h
  , float         const threshold
) {
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
    random_t&           random
  , rect_t        const rect
  , split_type    const type
  , positive<int> const min_w
  , positive<int> const min_h
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
