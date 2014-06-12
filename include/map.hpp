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

} //namespace yama
