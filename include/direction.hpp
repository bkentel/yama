#pragma once

#include "math.hpp"
#include <array>

namespace yama {

enum class direction {
    here, nw, n, ne, w, e, sw, s, se, up, down
};

enum class cardinal_direction {
    here, n, w, e, s 
};


struct direction_offsets {
    //                         here, nw,  n, ne,  w, e, sw, s, se, up, down
    std::array<int, 11> x {{0,    -1,  0,  1, -1, 1, -1, 0,  1,  0, 0}};
    std::array<int, 11> y {{0,    -1, -1, -1,  0, 0,  1, 1,  1,  0, 0}};
};

template <direction Dir>
inline grid_position_t relative_to(grid_position_t const p);

template <>
inline grid_position_t relative_to<direction::nw>(grid_position_t const p) {
    return {p.x - 1, p.y - 1};
}

template <>
inline grid_position_t relative_to<direction::n>(grid_position_t const p) {
    return {p.x, p.y - 1};
}

template <>
inline grid_position_t relative_to<direction::ne>(grid_position_t const p) {
    return {p.x + 1, p.y - 1};
}

template <>
inline grid_position_t relative_to<direction::sw>(grid_position_t const p) {
    return {p.x - 1, p.y + 1};
}

template <>
inline grid_position_t relative_to<direction::s>(grid_position_t const p) {
    return {p.x, p.y + 1};
}

template <>
inline grid_position_t relative_to<direction::se>(grid_position_t const p) {
    return {p.x + 1, p.y + 1};
}

template <>
inline grid_position_t relative_to<direction::e>(grid_position_t const p) {
    return {p.x + 1, p.y};
}

template <>
inline grid_position_t relative_to<direction::w>(grid_position_t const p) {
    return {p.x - 1, p.y};
}

inline point_t relative_to(point_t const p, direction const dir) {
    direction_offsets const off;
    auto const i = static_cast<int>(dir);
    return {p.x + off.x[i], p.y + off.y[i]};
}


//inline direction get_direction_ns(int x, int y) {
//    if (y > 0) {
//        return direction::s;
//    } else if (y < 0) {
//        return direction::n;
//    } else {
//        return direction::here;
//    }
//}
//
//inline direction get_direction_ew(int x, int y) {
//    if (x > 0) {
//        return direction::e;
//    } else if (x < 0) {
//        return direction::w;
//    } else {
//        return direction::here;
//    }
//}

} //namespace yama
