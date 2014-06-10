#pragma once

#include "assert.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! A value type with a restriction applied to it.
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Restriction>
class restricted_value {
public:
    static_assert(std::is_arithmetic<T>::value, "");

    restricted_value(T const value = T{0})
      : value_ { Restriction::apply(value) }
    {
        BK_ASSERT(Restriction::check(value));
    }

    bool operator<(T const rhs)  const { return value_ <  rhs; }
    bool operator<=(T const rhs) const { return value_ <= rhs; }
    bool operator>(T const rhs)  const { return value_ >  rhs; }
    bool operator>=(T const rhs) const { return value_ >= rhs; }

    restricted_value operator=(T const rhs) {
        BK_ASSERT(Restriction::check(rhs));

        value_ = Restriction::apply(rhs);
        return *this;
    }

    restricted_value operator!=(T const rhs) {
        return !(*this == rhs);
    }

    operator T() const { return value_; }
private:
    T value_;
};

namespace restriction {

////////////////////////////////////////////////////////////////////////////////
//! A value type restricted to [0, 100].
////////////////////////////////////////////////////////////////////////////////
struct restriction_percentage {
    template <typename T>
    static bool check(T const value) {
        constexpr auto lower = T{0};
        constexpr auto upper = T{100};

        return value >= lower && value <= upper;
    }

    template <typename T>
    static T apply(T const value) {
        constexpr auto lower = T{0};
        constexpr auto upper = T{100};

        return value > upper ? upper : value < lower ? lower : value;
    }
};

////////////////////////////////////////////////////////////////////////////////
//! A value type restricted to be no less than @tparam Min.
////////////////////////////////////////////////////////////////////////////////
template <int Min>
struct restriction_minimum {
    template <typename T>
    static bool check(T const value) {
        constexpr auto lower = T{Min};
        return value >= lower;
    }

    template <typename T>
    static T apply(T const value) {
        constexpr auto lower = T{Min};
        return value < lower ? lower : value;
    }
};

////////////////////////////////////////////////////////////////////////////////
//! A value type restricted to greater than 1.
////////////////////////////////////////////////////////////////////////////////
struct restriction_aspect_ratio {
    template <typename T>
    static bool check(T const value) {
        constexpr auto lower = T{1};
        return value >= lower;
    }

    template <typename T>
    static T apply(T const value) {
        constexpr auto lower = T{1};
        return value < lower ? lower : value;
    }
};

} //namespace restriction;

template <typename T>
using positive = restricted_value<T, restriction::restriction_minimum<0>>;
using aspect_ratio = restricted_value<float, restriction::restriction_aspect_ratio>;
using percentage = restricted_value<int, restriction::restriction_percentage>;

////////////////////////////////////////////////////////////////////////////////
//! A closed integral interval.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct closed_integral_interval {
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
        return value < lower ? lower : value > upper ? upper : value;
    }

    T lower;
    T upper;
};

template <typename T, typename U>
inline bool operator<(U const lhs, closed_integral_interval<T> const rhs) {
    return lhs < rhs.lower;
}

template <typename T, typename U>
inline bool operator<(closed_integral_interval<T> const lhs, U const rhs) {
    return lhs.upper < rhs;
}

////////////////////////////////////////////////////////////////////////////////
//! 2d point
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
//! 2d vector
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
//! axis aligned rectangle
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


////////////////////////////////////////////////////////////////////////////////
//! clamp value to [min, max]
////////////////////////////////////////////////////////////////////////////////
template <typename T>
inline T clamp(T value, T min, T max) {
    return value < min ? min : value > max ? max : value;
}

////////////////////////////////////////////////////////////////////////////////
//! 2d line segment
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct line2d {
    T x0, y0, x1, y1;
};

} //namespace yama
