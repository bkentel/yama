////////////////////////////////////////////////////////////////////////////////
//! @file
//! Mathematical functions and types.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "assert.hpp"

#include "checked_value.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! clamp @p value to [@p min, @p max].
////////////////////////////////////////////////////////////////////////////////
template <typename T>
inline T clamp(T const value, T const min, T const max) {
    return value < min ? min : value > max ? max : value;
}

namespace detail {

template <typename T, bool Fundamental> struct get_value_type;

template <typename T> struct get_value_type<T, true> {
    using type = T;
};

template <typename T> struct get_value_type<T, false> {
    using type = typename T::value_type;
};

} //namespace detail

template <typename T>
using get_value_type_t = typename detail::get_value_type<T, std::is_fundamental<T>::value>::type;

template <typename T, typename U>
using get_common_value_type_t = std::common_type_t<get_value_type_t<T>, get_value_type_t<U>>;

////////////////////////////////////////////////////////////////////////////////
//! A closed integral interval.
//!
//! @tparam T An integral type.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct closed_integral_interval {
    using value_type = get_value_type_t<T>;

    static_assert(std::is_integral<value_type>::value, "");

    closed_integral_interval(T const Lower, T const Upper)
      : lower {Lower}
      , upper {Upper}
    {
        BK_ASSERT(lower <= upper);
    }

    bool contains(T const value) const {
        return value >= lower && value <= upper;
    }

    T clamp(T const value) const {
        return yama::clamp(value, lower, upper);
    }

    T size() const {
        return upper - lower;
    }

    //TODO double
    float operator*(float f) const {
        BK_ASSERT(f >= 0.0f && f <= 1.0f);
        return static_cast<float>(lower) + static_cast<float>(size()) * f;
    }

    T lower;
    T upper;
};

template <typename T, typename U, typename R = get_common_value_type_t<T, U>>
inline closed_integral_interval<R>
make_closed_interval(T const lower, U const upper) {
    R const a = lower;
    R const b = upper;

    return closed_integral_interval<R>{a, b};
}

using positive_interval = closed_integral_interval<positive<int>>;

template <typename T>
inline bool operator<(get_value_type_t<T> const lhs, closed_integral_interval<T> const rhs) {
    return lhs < rhs.lower;
}

template <typename T, typename U>
inline bool operator<(closed_integral_interval<T> const lhs, get_value_type_t<T> const rhs) {
    return rhs < lhs;
}

template <typename T>
inline bool operator<=(get_value_type_t<T> const lhs, closed_integral_interval<T> const rhs) {
    return lhs <= rhs.lower;
}

template <typename T, typename U>
inline bool operator<=(closed_integral_interval<T> const lhs, get_value_type_t<T> const rhs) {
    return rhs <= lhs;
}

template <typename T>
inline bool operator>(get_value_type_t<T> const lhs, closed_integral_interval<T> const rhs) {
    return lhs > rhs.upper;
}

template <typename T, typename U>
inline bool operator>(closed_integral_interval<T> const lhs, get_value_type_t<T> const rhs) {
    return rhs > lhs;
}

template <typename T>
inline bool operator>=(get_value_type_t<T> const lhs, closed_integral_interval<T> const rhs) {
    return lhs >= rhs.upper;
}

template <typename T, typename U>
inline bool operator>=(closed_integral_interval<T> const lhs, get_value_type_t<T> const rhs) {
    return rhs >= lhs;
}

////////////////////////////////////////////////////////////////////////////////
//! clamp @p value to @p range.
////////////////////////////////////////////////////////////////////////////////
template <typename T>
inline T clamp(T const value, closed_integral_interval<T> const range) {
    return range.clamp(value);
}

////////////////////////////////////////////////////////////////////////////////
//! A two-dimensional point.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct point2d {
    point2d(T X, T Y) : x {X}, y {Y} {}
    point2d() : point2d {0, 0} {}

    T x;
    T y;
};

template <typename T>
bool operator==(point2d<T> const p, point2d<T> const q) {
    return p.x == q.x && p.y == q.y;
}

template <typename T>
bool operator!=(point2d<T> const p, point2d<T> const q) {
    return !(p == q);
}

////////////////////////////////////////////////////////////////////////////////
//! A two-dimensional vector.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct vector2d {
    T x;
    T y;

    template <typename R = float>
    R length() const {
        return static_cast<R>(std::sqrt(x*x + y*y));
    }

    template <typename R = float>
    vector2d<R> normalized() const {
        auto const len = length<R>();
        return {x / len, y / len};
    }
};

template <typename T>
inline vector2d<T> operator-(point2d<T> const p0, point2d<T> const p1) {
    return {p0.x - p1.x, p0.y - p1.y};
}

template <typename T>
inline vector2d<T> operator+(vector2d<T> const v0, vector2d<T> const v1) {
    return {v0.x + v1.x, v0.y + v1.y};
}

template <typename T>
inline vector2d<T> operator*(vector2d<T> const v, T const c) {
    return {v.x * c, v.y * c};
}

template <typename T>
inline vector2d<T> operator*(T const c, vector2d<T> const v) {
    return v * c;
}

////////////////////////////////////////////////////////////////////////////////
//! An axis-aligned rectangle.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct axis_aligned_rect {
    T left, top, right, bottom;

    axis_aligned_rect()
      : axis_aligned_rect {0, 0, 0, 0}
    {
    }

    axis_aligned_rect(T const Left, T const Top, T const Right, T const Bottom)
      : left {Left}, top {Top}, right {Right}, bottom {Bottom}
    {
    }

    explicit operator bool() const {
        return left < right && top < bottom;
    }

    T width() const { return right - left; }
    T height() const { return bottom - top; }

    T area() const { return width() * height(); }

    bool contains(T const x, T const y) const {
        return x >= left && y >= top && x < right && y < bottom;
    }

    bool contains(point2d<T> const p) const {
        return contains(p.x, p.y);
    }

    template <typename R = T>
    point2d<R> center() const {
        return {left + width() / R {2}, top + height() / R {2}};
    }

    bool is_top(point2d<T> const p) const { return p.y == top; }
    bool is_left(point2d<T> const p) const { return p.x == left; }
    bool is_bottom(point2d<T> const p) const { return p.y == bottom - 1; }
    bool is_right(point2d<T> const p) const { return p.x == right - 1; }

    bool is_top(T const x, T const y) const { return is_top(point2d<T> {x, y}); }
    bool is_left(T const x, T const y) const { return is_left(point2d<T> {{x, y}}); }
    bool is_bottom(T const x, T const y) const { return is_bottom(point2d<T> {{x, y}}); }
    bool is_right(T const x, T const y) const { return is_right(point2d<T> {{x, y}}); }

    bool is_border(point2d<T> const p) const {
        return is_top(p) || is_left(p) || is_right(p) || is_bottom(p);
    }

    bool is_border(T const x, T const y) const {
        return is_border(point2d<T> {x, y});
    }
};

template <typename T>
bool operator==(axis_aligned_rect<T> const a, axis_aligned_rect<T> const b) {
    return (a.left   == b.left)
        && (a.top    == b.top)
        && (a.right  == b.right)
        && (a.bottom == b.bottom);
}

template <typename T>
bool operator!=(axis_aligned_rect<T> const a, axis_aligned_rect<T> const b) {
    return !(a == b);
}

} //namespace yama
