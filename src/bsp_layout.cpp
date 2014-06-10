#include "pch.hpp"
#include "detail/bsp_layout_impl.hpp"

using bsp_layout = yama::bsp_layout;
using bsp_layout_impl = yama::detail::bsp_layout_impl;
using random_t = yama::random_t;

////////////////////////////////////////////////////////////////////////////////
//! Implemenation merely forwards bsp_layout_impl.
////////////////////////////////////////////////////////////////////////////////
class bsp_layout::impl_t : public bsp_layout_impl {
public:
    using bsp_layout_impl::bsp_layout_impl;
};
//==============================================================================
bsp_layout_impl::params_t
bsp_layout_impl::validate(params_t const params) {
    auto const& p = params;

    BK_ASSERT(p.map_w >= 16);
    BK_ASSERT(p.map_h >= 16);

    BK_ASSERT(p.min_room_w >= 0);
    BK_ASSERT(p.min_room_h >= 0);

    BK_ASSERT(p.border >= 0);

    return params;
}
//------------------------------------------------------------------------------
bsp_layout_impl::bsp_layout_impl(params_t const Params)
  : params_ {validate(Params)}
  , nodes_ {}
  , rooms_ {}
  , map_ {params_.map_w, params_.map_h}
{
}
//------------------------------------------------------------------------------
void bsp_layout_impl::clear() {
    nodes_.clear();
    rooms_.clear();
    map_.clear();
}
//------------------------------------------------------------------------------
yama::map bsp_layout_impl::generate(random_t& random) {
    clear();
    nodes_.push_back(node {rect_t {0, 0, params_.map_w, params_.map_h}});

    //each iteration can increase size()
    for (size_t i = 0; i < nodes_.size(); ++i) {
        split_node(random, nodes_[i]);
    }

    generate_rooms(random);

    for (auto const& n : nodes_) {
        if (n.is_leaf() && !n.is_empty()) {
            write_room(rooms_[n.get_data()]);
        }
    }

    connect(random, nodes_[0]);

    auto result = std::move(map_);
    map_ = map {params_.map_w, params_.map_h};

    return result;
}
//------------------------------------------------------------------------------
void bsp_layout_impl::split_node(random_t& random, node& n) {
    BK_ASSERT(n.is_empty());

    auto const min_w = params_.min_region_w;
    auto const min_h = params_.min_region_h;
    auto const rect  = n.bounds;

    if (!do_split(random, rect)) {
        return;
    }

    auto const type = generate::get_split_type(random, rect, min_w, min_h, params_.split_ratio);
    if (type == generate::split_type::none) {
        return;
    }

    auto const result = generate::split_rect(random, rect, type, min_w, min_h);
    auto const first  = std::get<1>(result);
    auto const second = std::get<2>(result);

    n.first  = nodes_.size() + 0;
    n.second = nodes_.size() + 1;

    nodes_.push_back(node {first});
    nodes_.push_back(node {second});
}
//------------------------------------------------------------------------------
void bsp_layout_impl::generate_rooms(random_t& random) {
    BK_ASSERT(rooms_.empty());

    for (auto& n : nodes_) {
        //internal node => next
        if (!n.is_leaf()) { continue; }

        auto const& bounds = n.bounds;

        //skip generation? => next
        if (!do_generate_room(random, bounds)) { continue; }

        auto const room = generate_room(random, bounds);

        std::cout << "room width=" << room.width() << " height=" << room.height() << std::endl;

        rooms_.emplace_back(room);
        n.set_data(rooms_.size() - 1);
    }
}
//------------------------------------------------------------------------------
bool bsp_layout_impl::do_split(random_t& random, yama::rect_t const bounds) const {
    auto const max_w = params_.max_region_w;
    auto const max_h = params_.max_region_h;

    auto const w = bounds.width();
    auto const h = bounds.height();

    if (w > max_w || h > max_h) {
        return true;
    }

    auto const float_w = static_cast<float>(w);
    auto const float_h = static_cast<float>(h);

    auto const ratio = w >= h ? float_w / float_h : float_h / float_w;

    if (ratio > params_.split_limit) {
        return true;
    } else {
        return params_.split_chance >= yama::random_uniform(random, 0, 100);
    }
}
//------------------------------------------------------------------------------
bool bsp_layout_impl::do_generate_room(random_t& random, yama::rect_t const bounds) const {
    if (bounds.width()  < params_.min_room_w
        || bounds.height() < params_.min_room_h
    ) {
        return false;
    }

    return params_.room_generation_rate >= yama::random_uniform(random, 0, 100);
}
//------------------------------------------------------------------------------
yama::rect_t bsp_layout_impl::generate_room(random_t& random, yama::rect_t bounds) const {
    auto const border = params_.border;

    bounds.left += border;
    bounds.top  += border;

    auto const min_w = params_.min_room_w;
    auto const min_h = params_.min_room_h;

    auto const max_w = bounds.width();
    auto const max_h = bounds.height();

    auto const weight = 90;

    auto const weighted_max_w = std::max(min_w, (max_w*(100 + weight)) / 100);
    auto const weighted_max_h = std::max(min_h, (max_h*(100 + weight)) / 100);

    auto const w = std::min(max_w, random_uniform(random, min_w, weighted_max_w));
    auto const h = std::min(max_h, random_uniform(random, min_h, weighted_max_h));

    return generate::rect(random, bounds, w, h);
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

//------------------------------------------------------------------------------
//void bsp_layout_impl::do_connect(
//    random_t&          random
//  , yama::rect_t const bounds
//  , yama::rect_t const first
//  , yama::rect_t const second
//) {
//    auto const end_i = second.center<int>();
//    auto const end_f = second.center<float>();
//
//    auto p = first.center();
//
//    static auto const as_float = [](point2d<int> const p) {
//        return point2d<float> {static_cast<float>(p.x), static_cast<float>(p.y)};
//    };
//
//    auto const get_vector = [&random, end_f](point2d<float> const p) {
//        std::uniform_real<float> dist;
//
//        auto const v = end_f - p;
//        auto const r = 4.0f*yama::vector2d<float> {dist(random), dist(random)};
//
//        return (v + r).normalized();
//    };
//
//    auto const write_corridor = [this, &p, bounds](int& value, int delta) {
//        auto const step = delta >= 0 ? 1 : -1;
//
//        while (delta && bounds.contains(p)) {
//            delta -= step;
//            value += step;
//            map_.set<map::property::category>(p.x, p.y, tile_category::corridor);
//        }
//    };
//
//    int count = 0;
//
//    while (count++ < 100 && !second.contains(p)) {
//        BK_ASSERT(bounds.contains(p));
//
//        auto const distance = (end_i - p).length<int>();
//        auto const step = static_cast<float>(random_uniform(random, 1, distance));
//        auto const v = get_vector(as_float(p)) * step;
//
//        auto dx = static_cast<int>(std::round(v.x));
//        auto dy = static_cast<int>(std::round(v.y));
//
//        //zig-zag, or zag-zig?
//        if (random_bool(random)) {
//            p = make_connection_tunnel(p, bounds, dx, 0);
//            p = make_connection_tunnel(p, bounds,  0, dy);
//        } else {
//            p = make_connection_tunnel(p, bounds,  0, dy);
//            p = make_connection_tunnel(p, bounds, dx, 0);
//        }
//    }
//}

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

        auto dist = std::uniform_int_distribution<>{-range, range};

        auto const v = end - point;
        auto const r = yama::vector2d<int> {dist(random), dist(random)};

        return (v + r);
    };

    int count = 0;
    while (!second.contains(p)) {
        //BK_ASSERT(bounds.contains(p));

        auto const step = random_uniform(random, params_.corridor_seg_len_min, params_.corridor_seg_len_max);
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

//==============================================================================
bsp_layout::bsp_layout(params_t p)
  : impl_ {std::make_unique<impl_t>(p)}
{
}
//------------------------------------------------------------------------------
bsp_layout::~bsp_layout() {
}
//------------------------------------------------------------------------------
bsp_layout::params_t bsp_layout::params() const {
    return impl_->params();
}
//------------------------------------------------------------------------------
void bsp_layout::set_params(params_t const p) {
    impl_->set_params(p);
}
//------------------------------------------------------------------------------
yama::map yama::bsp_layout::generate(random_t& random) {
    return impl_->generate(random);
}
//------------------------------------------------------------------------------
