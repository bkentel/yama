#pragma once

#include "tile.hpp"

namespace yama {

namespace detail {

enum class map_property {
    category, room_id, texture_id
};

template <map_property P>
struct map_property_return_type;

template <> struct map_property_return_type<map_property::category> {
    using type = tile_category;
};

} //namespace detail

class map {
public:
    using property = detail::map_property;

    map(int width, int height);
    ~map();

    map(map&& other);
    map& operator=(map&& rhs);

    void clear();

    template <property P, typename R>
    void set(int x, int y, R value);

    template <property P, typename R>
    void set(grid_position_t p, R value);

    template <property P>
    typename detail::map_property_return_type<P>::type
    get(int x, int y) const;

    template <property P>
    typename detail::map_property_return_type<P>::type
    get(grid_position_t p) const;

    bool is_valid_position(int x, int y) const;

    bool is_valid_position(grid_position_t p) const {
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
inline void map::set<map::property::category, tile_category>(int const x, int const y, tile_category const value) {
    set_category_(x, y, value);
}

template <>
tile_category
inline map::get<map::property::category>(int const x, int const y) const {
    return get_category_(x, y);
}

template <>
tile_category
inline map::get<map::property::category>(grid_position_t const p) const {
    return get_category_(p.x, p.y);
}

} //namespace yama
