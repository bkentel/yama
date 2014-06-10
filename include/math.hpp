#pragma once

#include "assert.hpp"

namespace yama {

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