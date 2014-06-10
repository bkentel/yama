#pragma once

#include <vector>

#include "bsp_layout.hpp"
#include "grid.hpp"
#include "math.hpp"
#include "random.hpp"
#include "generate.hpp"
#include "direction.hpp"

namespace yama {

namespace detail {

class bsp_layout_impl {
public:
    using params_t = bsp_layout::params_t;

    struct node {
        using index_t = int;
        static index_t const EMPTY_VALUE = -1;

        explicit node(rect_t const Bounds)
          : first  {EMPTY_VALUE}
          , second {EMPTY_VALUE}
          , bounds {Bounds}
        {
        }

        bool is_leaf() const {
            return first == EMPTY_VALUE;
        }

        bool is_empty() const {
            return first == EMPTY_VALUE && second == EMPTY_VALUE;
        }

        void set_data(index_t i) {
            BK_ASSERT(is_leaf());
            second = i;
        }

        index_t get_data() const {
            BK_ASSERT(is_leaf());
            return second;
        }

        index_t first;
        index_t second;
        rect_t  bounds;
    };

    static params_t validate(params_t params);
public:
    //! construct with a (default) param set.
    explicit bsp_layout_impl(params_t params = params_t {});

    //! get the current param set.
    params_t params() const {
        return params_;
    }

    //! set the current param set.
    void set_params(params_t const p) {
        params_ = validate(p);
    }

    //! reset internal state and keep the current param set.
    void clear();

    //! generate a new map
    map generate(random_t& random);

    //! decide whether to split a node.
    bool do_split(random_t& random, rect_t bounds) const;

    //! decide whether to generate a room.
    bool do_generate_room(random_t& random, rect_t bounds) const;

    //! split a node into sub nodes
    void split_node(random_t& random, node& n);

    //! generate a room.
    rect_t generate_room(random_t& random, rect_t bounds) const;

    //! rasterize a room to the final map (grid).
    void write_room(rect_t room);

    //! fill the bsp tree with rooms
    void generate_rooms(random_t& random);

    ////////////////////////////////////////////////////////////////////////////
    //! Connect two rects contained within bounds.
    //!
    //!
    ////////////////////////////////////////////////////////////////////////////
    void do_connect(random_t& random, rect_t bounds, rect_t first, rect_t second);

    enum class possible {
        no, yes, maybe
    };

    possible can_tunnel(point_t const left, point_t const ahead, point_t const right) const {
        auto const is_wall = [&](point_t const p) {
            constexpr auto cat = map::property::category;
            return map_.get<cat>(p.x, p.y) == tile_category::wall;
        };

        if (!map_.is_valid_position(ahead)) {
            return possible::no;
        } else if (!is_wall(ahead)) {
            return possible::yes;
        }

        auto const ok_left  = map_.is_valid_position(left.x,  left.y);
        auto const ok_right = map_.is_valid_position(right.x, right.y);

        if (!(ok_left && ok_right)) {
            return possible::no;
        }

        if (is_wall(left) && is_wall(right)) {
            return possible::maybe;
        }

        return possible::no;
    }

    template <direction Left, direction Ahead, direction Right>
    static std::tuple<point_t, point_t, point_t> get_ahead(point_t const p) {
        return std::make_tuple(relative_to<Left>(p), relative_to<Ahead>(p), relative_to<Right>(p));
    }

    static tile_category corridor_transform(tile_category const value) {
        using cat = yama::tile_category;

        switch (value) {
        case cat::empty: return cat::corridor;
        case cat::wall:  return cat::door;
        default :        return value;
        }
    }

    point_t make_connection_tunnel(
        point_t p
      , rect_t //bounds
      , int dx
      , int dy
    ) {
        BK_ASSERT(!(dx && dy));

        auto const get_n = [](point_t const point) {
            return get_ahead<direction::nw, direction::n, direction::ne>(point);
        };

        auto const get_s = [](point_t const point) {
            return get_ahead<direction::sw, direction::s, direction::se>(point);
        };

        auto const get_e = [](point_t const point) {
            return get_ahead<direction::ne, direction::e, direction::se>(point);
        };

        auto const get_w = [](point_t const point) {
            return get_ahead<direction::nw, direction::w, direction::sw>(point);
        };

        using get_f = std::tuple<point_t, point_t, point_t> (point_t);

        auto const check = [this, &p](get_f get) {
            auto ahead  = get(p);
            auto result = can_tunnel(std::get<0>(ahead), std::get<1>(ahead), std::get<2>(ahead));

            if (result == possible::maybe) {
                ahead = get(std::get<1>(ahead));
                result = can_tunnel(std::get<0>(ahead), std::get<1>(ahead), std::get<2>(ahead));

                return (result != possible::no);
            }

            return (result == possible::yes);
        };

        auto const step_x = dx ? (dx > 0 ? 1 : -1) : 0;
        auto const step_y = dy ? (dy > 0 ? 1 : -1) : 0;

        auto const ok = [&] {
            if (step_y > 0) {
                return check(get_s);
            } else if (step_y < 0) {
                return check(get_n);
            } else if (step_x > 0) {
                return check(get_e);
            } else if (step_x < 0) {
                return check(get_w);
            } else {
                return false;
            }
        };

        while ((dx || dy) && ok()) {
            p.x += step_x;
            p.y += step_y;

            dx -= step_x;
            dy -= step_y;

            constexpr auto cat = map::property::category;
            auto const value = map_.get<cat>(p.x, p.y);
            map_.set<cat>(p.x, p.y, corridor_transform(value));
        }

        return p;
    }

    ////////////////////////////////////////////////////////////////////////////
    //! Recursively connect all rooms depth-first.
    //!
    //! @return {has_room, bounds} where @p bounds are the room itself if has_room
    //! is true, otherise @p bounds are the region's bounds.
    ////////////////////////////////////////////////////////////////////////////
    std::pair<bool, rect_t>
    connect(random_t& random, node const& n) {
        //base case
        if (n.is_leaf()) {
            //if not empty, return the room's rect;
            //otherise return the region's bounds
            auto const has_room = !n.is_empty();
            return {has_room, has_room ? rooms_[n.get_data()] : n.bounds};
        }

        //recurse
        auto const a = connect(random, nodes_[n.first]);
        auto const b = connect(random, nodes_[n.second]);

        if (a.first && b.first) {
            //both children are connected; connect the children themselves
            do_connect(random, n.bounds, a.second, b.second);
            //choose a random child to return as the result
            return {true, random_bool(random) ? a.second : b.second};
        } else if (a.first && !b.first) {
            //one child is empty; return the other
            return {true, a.second};
        } else if (!a.first && b.first) {
            //one child is empty; return the other
            return {true, b.second};
        }

        //both children are empty
        return {false, n.bounds};
    }

    params_t            params_;
    std::vector<node>   nodes_;
    std::vector<rect_t> rooms_;
    yama::map           map_;
};
//
//class bsp_layout_impl {
//public:
//    using params_t = bsp_layout::params_t;
//
//    struct region {
//        bool is_leaf() const {
//            return first == 0;
//        }
//
//        bool has_data() const {
//            return is_leaf() && second != 0;
//        }
//
//        void split(size_t first, size_t second) {
//            BK_ASSERT(is_leaf() && !has_data());
//            BK_ASSERT(first && second);
//
//            this->first = first;
//            this->second = second;
//        }
//
//        void set_data(size_t data) {
//            BK_ASSERT(is_leaf());
//            second = data;
//        }
//
//        size_t get_data() const {
//            BK_ASSERT(has_data());
//            return second;
//        }
//
//        rect_t rect;   //! bounds for the subregion
//        size_t first;  //! index of the first child region; 0 if a leaf
//        size_t second; //! index of the first child region; 0 if a leaf and empty;
//    };
//public:
//    explicit bsp_layout_impl(params_t p = params_t {})
//      : params_ {p}
//      , cat_grid_ {p.map_w, p.map_h}
//    {
//    }
//
//    params_t params() const { return params_; }
//    void set_params(params_t p = params_t {}) { params_ = p; }
//
//    map generate(random_t& random);
//
//    rect_t generate_room(random_t& random, rect_t bounds) const;
//    bool skip_generate_room(random_t& random, rect_t region) const;
//    grid_position_t find_empty_room_pos(random_t& random, rect_t rect) const;
//private:
//    params_t params_;
//
//    grid<tile_category> cat_grid_;
//};

} //namespace detail
} //namespace yama
