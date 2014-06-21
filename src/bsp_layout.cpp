#include "pch.hpp"
#include "detail/bsp_layout_impl.hpp"

using bsp_layout = yama::bsp_layout;
using bsp_layout_impl = yama::detail::bsp_layout_impl;
using random_t = yama::random_t;

////////////////////////////////////////////////////////////////////////////////
//! Implementation merely forwards bsp_layout_impl.
////////////////////////////////////////////////////////////////////////////////
class bsp_layout::impl_t : public bsp_layout_impl {
public:
    using bsp_layout_impl::bsp_layout_impl;
};

//==============================================================================
// yama::detail::bsp_layout_impl
//==============================================================================
bsp_layout_impl::params_t&
bsp_layout_impl::validate(params_t& params) {
    return params;
}
//------------------------------------------------------------------------------
bsp_layout_impl::bsp_layout_impl(params_t params)
  : params_ {validate(params)}
  , nodes_ {}
{
}
//------------------------------------------------------------------------------
void bsp_layout_impl::clear() {
    nodes_.clear();
}
//------------------------------------------------------------------------------
void bsp_layout_impl::generate(
    random_t&           random
  , yama::map&          map
  , on_create_room_t&   on_create_room
  , on_connect_rooms_t& on_connect_rooms
) {
    clear();

    auto const map_rect = rect_t {0, 0, map.width(), map.height()};
    nodes_.push_back(node_t {map_rect});

    generate_tree(random);
    generate_rooms(random, map, on_create_room);

    connect(random, on_connect_rooms, nodes_[0]);
}
//------------------------------------------------------------------------------
void bsp_layout_impl::generate_tree(random_t& random) {
    BK_ASSERT(nodes_.size() == 1);

    //each iteration can increase size()
    for (size_t i = 0; i < nodes_.size(); ++i) {
        split_node(random, nodes_[i]);
    }
}
//------------------------------------------------------------------------------
void bsp_layout_impl::split_node(random_t& random, node_t& node) {
    BK_ASSERT(node.is_empty());

    auto const min_w = params_.region_w_range.lower;
    auto const min_h = params_.region_h_range.lower;
    auto const rect  = node.bounds;

    if (!do_split(random, rect)) {
        return;
    }

    auto const type = generate::get_split_type(random, rect, min_w, min_h, params_.split_aspect);
    if (type == generate::split_type::none) {
        return;
    }

    auto const result = generate::split_rect(random, rect, type, min_w, min_h);
    auto const first  = std::get<1>(result);
    auto const second = std::get<2>(result);

    node.first  = nodes_.size() + 0;
    node.second = nodes_.size() + 1;

    nodes_.push_back(node_t {first});
    nodes_.push_back(node_t {second});
}
//------------------------------------------------------------------------------
void bsp_layout_impl::generate_rooms(
    random_t&         random
  , yama::map&        map
  , on_create_room_t& on_create_room
) {
    for (auto& node : nodes_) {
        if (!node.is_leaf()) {
            continue; //internal node => next
        }

        on_create_room(random, map, node.bounds);
    }
}
//------------------------------------------------------------------------------
bool bsp_layout_impl::do_split(
    random_t&          random
  , yama::rect_t const bounds
) const {
    auto const max_w = params_.region_w_range.upper;
    auto const max_h = params_.region_h_range.upper;

    auto const w = bounds.width();
    auto const h = bounds.height();

    if (w > max_w || h > max_h) {
        return true;
    }

    auto const float_w = static_cast<float>(w);
    auto const float_h = static_cast<float>(h);

    auto const ratio = (w >= h)
      ? (float_w / float_h)
      : (float_h / float_w);

    return (ratio > params_.split_limit_aspect)
      ? true
      : (params_.region_split_chance >= random_percent(random));
}
//------------------------------------------------------------------------------
bool bsp_layout_impl::do_generate_room(
    random_t&          random
  , yama::rect_t const bounds
) const {
    if (bounds.width()  < params_.room_w_range
     || bounds.height() < params_.room_h_range
    ) {
        return false;
    }

    return params_.room_generation_chance >= random_percent(random);
}
//------------------------------------------------------------------------------
yama::rect_t bsp_layout_impl::generate_room(random_t& random, yama::rect_t bounds) const {
    auto const& p = params_;
    return generate::weighted_bounded_rect(
        random
      , bounds
      , p.room_w_range.lower
      , p.room_h_range.lower
      , p.room_size_weight
      , p.room_size_variance
      , p.border_size
    );
}
//------------------------------------------------------------------------------
void bsp_layout_impl::write_room(yama::rect_t const room) {
    auto const map_bounds = rect_t {0, 0, params_.map_w, params_.map_h};

    auto const is_wall_or_door = [&](grid_position_t const p) {
        if (!map_bounds.contains(p)) {
            return false;
        }

        auto const value = map_.get<map::property::category>(p);
        return value == tile_category::wall || value == tile_category::door;
    };

    auto const check = [&](std::tuple<grid_position_t, grid_position_t, grid_position_t> const t) {
        return is_wall_or_door(std::get<0>(t))
            && is_wall_or_door(std::get<1>(t))
            && is_wall_or_door(std::get<2>(t));
    };

    auto const omit_left = [&](grid_position_t const p) {
        return check(get_ahead<direction::nw, direction::w, direction::sw>(p));
    };

    auto const omit_top = [&](grid_position_t const p) {
        return check(get_ahead<direction::nw, direction::n, direction::ne>(p));
    };

    auto const omit_right = [&](grid_position_t const p) {
        return check(get_ahead<direction::ne, direction::e, direction::se>(p));
    };

    auto const omit_bottom = [&](grid_position_t const p) {
        return check(get_ahead<direction::sw, direction::s, direction::se>(p));
    };

    yama::for_each_xy(room, [&](grid_position_t const p) {
        auto const is_border = room.is_border(p);

        auto const is_l = room.is_left(p);
        auto const is_t = room.is_top(p);
        auto const is_r = room.is_right(p);
        auto const is_b = room.is_bottom(p);

        BK_ASSERT(!(is_l && is_r));
        BK_ASSERT(!(is_t && is_b));

        auto const omit_wall =
            (is_l ? omit_left(p)   : true)
         && (is_t ? omit_top(p)    : true)
         && (is_r ? omit_right(p)  : true)
         && (is_b ? omit_bottom(p) : true);

        auto const value = is_border
          ? omit_wall
              ? tile_category::floor
              : tile_category::wall
          : tile_category::floor;

        map_.set<map::property::category>(p.x, p.y, value);
    });
}
//------------------------------------------------------------------------------
void bsp_layout_impl::do_connect(
    random_t&          random
  , yama::rect_t const bounds
  , yama::rect_t const first
  , yama::rect_t const second
) {
    auto       p   = first.center();
    auto const beg = p;
    auto const end = second.center();

    auto const get_vector = [&](point_t const point, int const n) {
        auto const range = static_cast<int>(
            std::round(static_cast<float>(n) * params_.corridor_randomness)
        );

        auto const x = random_uniform(random, -range, range);
        auto const y = random_uniform(random, -range, range);

        auto const v = end - point;
        auto const r = yama::vector2d<int> {x, y};

        return (v + r);
    };

    auto const seg_len_range = params_.corridor_segment_length_range;

    int count = 0;
    while (!second.contains(p)) {
        int const step = random_uniform(random, seg_len_range);
        auto const v = get_vector(p, count);

        auto dx = v.x >= 0 ? std::min(v.x, step) : std::max(v.x, -step);
        auto dy = v.y >= 0 ? std::min(v.y, step) : std::max(v.y, -step);

        //zig-zag, or zag-zig?
        if (random_bool(random)) {
            p = make_connection_tunnel(p, bounds, dx,  0);
            p = make_connection_tunnel(p, bounds,  0, dy);
        } else {
            p = make_connection_tunnel(p, bounds,  0, dy);
            p = make_connection_tunnel(p, bounds, dx,  0);
        }

        count++;

        if (count == 100) {
            std::cout << "warning!" << std::endl;
            break;
        } else if (count % 20 == 0) {
            p = beg;
        }
    }
}
//------------------------------------------------------------------------------
bsp_layout_impl::possible
bsp_layout_impl::can_tunnel(
    yama::point_t const left
  , yama::point_t const ahead
  , yama::point_t const right
) const {
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
//------------------------------------------------------------------------------
yama::point_t bsp_layout_impl::make_connection_tunnel(
    yama::point_t p
  , yama::rect_t //bounds
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
//------------------------------------------------------------------------------
yama::tile_category
bsp_layout_impl::corridor_transform(
    yama::tile_category const value
) {
    using cat = yama::tile_category;

    switch (value) {
    case cat::empty: return cat::corridor;
    case cat::wall:  return cat::door;
    default :        return value;
    }
}
//------------------------------------------------------------------------------
std::pair<bool, yama::rect_t>
bsp_layout_impl::connect(
    random_t&           random
  , on_connect_rooms_t& on_connect_rooms
  , node_t const&       node
) {
    //base case
    if (node.is_leaf()) {
        //if not empty, return the room's rect;
        //otherise return the region's bounds
        auto const has_room = !node.is_empty();
        return {has_room, has_room ? rooms_[node.get_data()] : n.bounds};
    }

    //recurse
    auto const a = connect(random, on_connect_rooms, nodes_[node.first]);
    auto const b = connect(random, on_connect_rooms, nodes_[node.second]);

    if (a.first && b.first) {
        //both children are connected; connect the children themselves
        do_connect(random, node.bounds, a.second, b.second);
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
    return {false, node.bounds};
}

//==============================================================================
// yama::bsp_layout
//==============================================================================
bsp_layout::bsp_layout(params_t p)
  : impl_ {std::make_unique<impl_t>(p)}
{
}
//------------------------------------------------------------------------------
bsp_layout::~bsp_layout() {
}
//------------------------------------------------------------------------------

void yama::bsp_layout::generate(
    random_t&           random
  , yama::map&          map
  , on_create_room_t&   on_create_room
  , on_connect_rooms_t& on_connect_rooms
) {
    impl_->generate(random, map, on_create_room, on_connect_rooms);
}
//------------------------------------------------------------------------------

