#pragma once

#include <vector>
#include <algorithm>

#include "assert.hpp"
#include "types.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! 2D grid of Ts
////////////////////////////////////////////////////////////////////////////////
template <typename T>
class grid {
public:
    grid(int const Width, int const Height, T const value = {})
      : width_  {Width}
      , height_ {Height}
      , data_   {}
    {
        BK_ASSERT(width_ > 0 && height_ > 0);
        data_.resize(width_*height_, value);
    }

    bool is_valid_index(int const x, int const y) const {
        return (x >= 0 && x < width_)
            && (y >= 0 && y < height_);
    }

    bool is_valid_index(grid_position_t const p) const {
        return is_valid_index(p.x, p.y);
    }

    void clear(T const value = T {}) {
        std::fill(std::begin(data_), std::end(data_), value);
    }

    T operator()(int const x, int const y) const {
        return data_[index_of_(x, y)];
    }

    T& operator()(int const x, int const y) {
        return data_[index_of_(x, y)];
    }

    T operator[](grid_position_t const p) const {
        return (*this)(p.x, p.y);
    }

    T& operator[](grid_position_t const p) {
        return (*this)(p.x, p.y);
    }

//    T at_or(int const x, int const y, T const value = T {}) const {
//        if (is_valid_index(x, y)) {
//            return value;
//        } else {
//            return at(x, y);
//        }
//    }
//
//    T at_or(grid_position_t const p, T const value = T {}) const {
//        return at_or(p.x, p.y, value);
//    }

    int width() const { return width_; }
    int height() const { return height_; }
private:
    size_t index_of_(int x, int y) const {
        BK_ASSERT(is_valid_index(x, y));
        return x + y*width_;
    }

    int width_;
    int height_;

    std::vector<T> data_;
};

template <typename T, typename F>
void for_each_xy(T&& grid, F&& function) {
    auto const w = grid.width();
    auto const h = grid.height();

    for (auto yi = 0; yi < h; ++yi) {
        for (auto xi = 0; xi < w; ++xi) {
            function(xi, yi, grid(xi, yi));
        }
    }
}

template <typename T, typename F>
void for_each_xy(T x0, T y0, T x1, T y1, F function) {
    for (auto yi = y0; yi < y1; ++yi) {
        for (auto xi = x0; xi < x1; ++xi) {
            function(xi, yi);
        }
    }
}

template <typename T, typename F>
void for_each_xy(axis_aligned_rect<T> rect, F function) {
    for (auto yi = rect.top; yi < rect.bottom; ++yi) {
        for (auto xi = rect.left; xi < rect.right; ++xi) {
            function(grid_position_t {xi, yi});
        }
    }
}


} //namespace yama
