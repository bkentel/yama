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
    ////////////////////////////////////////////////////////////////////////////
    void do_connect(random_t& random, rect_t bounds, rect_t first, rect_t second);

    ////////////////////////////////////////////////////////////////////////////
    //! 3-state logic of sorts.
    ////////////////////////////////////////////////////////////////////////////
    enum class possible {
        no, yes, maybe
    };

    ////////////////////////////////////////////////////////////////////////////
    //! Given three co-linear points, return whether creating a tunnel at
    //! @p ahead is possible.
    ////////////////////////////////////////////////////////////////////////////
    possible can_tunnel(point_t left, point_t ahead, point_t right) const;

    ////////////////////////////////////////////////////////////////////////////
    //! Get a tuple of three co-linear points relative to @p p.
    ////////////////////////////////////////////////////////////////////////////
    template <direction Left, direction Ahead, direction Right>
    static std::tuple<point_t, point_t, point_t> get_ahead(point_t const p) {
        return std::make_tuple(relative_to<Left>(p), relative_to<Ahead>(p), relative_to<Right>(p));
    }

    ////////////////////////////////////////////////////////////////////////////
    //! Transform value given that a corridor passes over it.
    ////////////////////////////////////////////////////////////////////////////
    static tile_category corridor_transform(tile_category value);

    ////////////////////////////////////////////////////////////////////////////
    //! Make a tunnel segment of length abs(dx) or abs(dy).
    ////////////////////////////////////////////////////////////////////////////
    point_t make_connection_tunnel(
        point_t p, rect_t bounds, int dx, int dy
    );

    ////////////////////////////////////////////////////////////////////////////
    //! Recursively connect all rooms depth-first.
    //!
    //! @return {has_room, bounds} where @p bounds are the room itself if has_room
    //! is true, otherise @p bounds are the region's bounds.
    ////////////////////////////////////////////////////////////////////////////
    std::pair<bool, rect_t> connect(random_t& random, node const& n);

    params_t            params_;
    std::vector<node>   nodes_;
    std::vector<rect_t> rooms_;
    yama::map           map_;
};

} //namespace detail
} //namespace yama
