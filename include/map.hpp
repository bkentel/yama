#pragma once

#include "tile.hpp"
#include "math.hpp"

namespace yama {

static constexpr int map_min_size = 10;
using map_size = checked_value<int, check_minimum<int, map_min_size>>;

enum class map_property {
    category, room_id, texture_id
};

namespace detail {

template <map_property P> struct property_mapping;

template <>
struct property_mapping<map_property::category> {
    static auto const property = map_property::category;
    using type = tile_category;
};

} //namespace detail

class map {
public:
    using property = map_property;

    template <map_property Property>
    using mapping_t = typename detail::property_mapping<Property>::type;

    map(map_size width, map_size height);
    ~map();

    map(map&& other);
    map& operator=(map&& rhs);

    void clear();

    //!
    template <property P>
    void set(int x, int y, mapping_t<P> value);

    //!
    template <property P>
    void set(grid_position_t p, mapping_t<P> value);

    //!
    template <property P>
    mapping_t<P> get(int x, int y) const;

    //!
    template <property P>
    mapping_t<P> get(grid_position_t p) const;

    //!
    bool is_valid_position(int x, int y) const;

    //!
    bool is_valid_position(grid_position_t const p) const {
        return is_valid_position(p.x, p.y);
    }

    int width() const;
    int height() const;
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;

    tile_category get_category_(int x, int y) const;
    void set_category_(int x, int y, tile_category value);
};

template <>
inline void map::set<map_property::category>(int const x, int const y, tile_category const value) {
    set_category_(x, y, value);
}

template <>
inline void map::set<map_property::category>(grid_position_t const p, tile_category const value) {
    set_category_(p.x, p.y, value);
}

template <>
inline tile_category
map::get<map_property::category>(int const x, int const y) const {
    return get_category_(x, y);
}

template <>
inline tile_category
map::get<map_property::category>(grid_position_t const p) const {
    return get_category_(p.x, p.y);
}

////////////////////////////////////////////////////////////////////////////////

//TODO move elsewhere
inline boost::tribool can_tunnel(
    yama::map       const& map
  , grid_position_t const  left
  , grid_position_t const  ahead
  , grid_position_t const  right
) {
    auto const is_wall = [&](point_t const p) {
        constexpr auto cat = map::property::category;
        return map.get<cat>(p.x, p.y) == tile_category::wall;
    };

    if (!map.is_valid_position(ahead)) {
        return false;
    }

    if (!is_wall(ahead)) {
        return true;
    }

    auto const ok_left  = map.is_valid_position(left);
    auto const ok_right = map.is_valid_position(right);

    auto const is_wall_left  = ok_left  && is_wall(left);
    auto const is_wall_right = ok_right && is_wall(right);

    if (is_wall_left && !is_wall_right) {
        return true;
    } else if (!is_wall_left && is_wall_right) {
        return true;
    } else if (!is_wall_left && !is_wall_right) {
        return false;
    }

    return boost::indeterminate;
}

template <typename T> inline
int signum(T const x, std::false_type) {
    return T{0} < x;
}

template <typename T> inline
int signum(T const x, std::true_type) {
    return (T{0} < x) - (x < T{0});
}

template <typename T> inline
int signum(T const x) {
    return signum(x, std::is_signed<T>{});
}

template <typename T, typename Function>
inline void for_each_integer(T begin, T delta, Function&& function) {
    auto const sign  = signum(delta);
    auto const count = sign*delta;

    for (auto i = decltype(count){0}; i < count; ++i) {
        if (!function(begin)) {
            return;
        }

        begin += sign;
    }
}

template <
    unsigned b0, unsigned b1, unsigned b2,
    unsigned b3, unsigned b4, unsigned b5,
    unsigned b6, unsigned b7, unsigned b8
>
struct rule {
    using flip_x = rule<
        b6, b7, b8
      , b3, b4, b5
      , b0, b1, b2
    >;

    using flip_y = rule<
        b2, b1, b0
      , b5, b4, b3
      , b8, b7, b6
    >;

    using flip_xy = rule<
        b8, b7, b6
      , b5, b4, b3
      , b2, b1, b0
    >;

    enum : unsigned { value =
        (b0 << 0) | (b1 << 1) | (b2 << 2)
      | (b3 << 3) | (b4 << 4) | (b5 << 5)
      | (b6 << 6) | (b7 << 7) | (b8 << 8)
    };
};

inline bool can_dig_corridor(yama::map const& map, grid_position_t const p) {
    auto const get = [&](int const dx, int const dy) {
        constexpr auto cat  = map::property::category;
        constexpr auto wall = tile_category::wall;

        return map.is_valid_position(p.x + dx, p.y + dy)
           && (map.get<cat>(p.x + dx,p. y + dy) == wall);
    };

    //[0][1][2]
    //[3][4][5]
    //[6][7][8]
    bool const cell[] = {
        get(-1, -1), get(0, -1), get(1, -1)
      , get(-1,  0), get(0,  0), get(1,  0)
      , get(-1,  1), get(0,  1), get(1,  1)
    };

    if (!cell[4]) {
        return true;
    }

    if ((!cell[0] && cell[1] && cell[3])
     || (!cell[2] && cell[1] && cell[5])
     || (!cell[6] && cell[3] && cell[7])
     || (!cell[8] && cell[5] && cell[7])
    ) {
        return false;
    }

    return true;
}

inline grid_position_t write_corridor_segment(
    yama::map&            map
  , grid_position_t const start
  , int const             dx
  , int const             dy
) {
    BK_ASSERT(dx ^ dy);

    if (dx) {
        for_each_integer(start.x, dx, [y = start.y, &map](int const x) {
            if (!can_dig_corridor(map, grid_position_t {x, y})) {
                return false;
            }



            return true;
        });

        if (dx > 0) {
            //east
        } else {
            //west
        }
    } else {
        if (dy > 0) {
            //south
        } else {
            //north
        }
    }

}

} //namespace yama
