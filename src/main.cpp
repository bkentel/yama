#include "pch.hpp"
#include "assert.hpp"
#include "types.hpp"
#include "client.hpp"
#include "random.hpp"
#include "algorithm.hpp"
#include "renderer.hpp"

namespace yama {


enum class direction {
    here, nw, n, ne, w, e, sw, s, se, up, down
};

using texture_src = axis_aligned_rect<uint16_t>;

using room_id = uint16_t;

inline grid_position_t relative_pos(direction dir, grid_position_t pos) {
    //                       here, nw,  n, ne, w,  e, sw, s, se, up, down
    static int const dx[] = {0,    -1,  0,  1, -1, 1, -1, 0, 1,   0, 0};
    static int const dy[] = {0,    -1, -1, -1,  0, 0,  1, 1, 1,   0, 0};

    auto const i = static_cast<size_t>(dir);

    pos.x += dx[i];
    pos.y += dy[i];

    return pos;
}




////////////////////////////////////////////////////////////////////////////////
//! BSP tree based map layour generator.
////////////////////////////////////////////////////////////////////////////////
class bsp_layout {
public:
    ////////////////////////////////////////////////////////////////////////////
    //! BSP layout generation paramaters.
    ////////////////////////////////////////////////////////////////////////////
    struct params {
        int width  = 64; //width of the root
        int height = 64; //height of the root
        int min_w  = 4;  //minimum width for children
        int min_h  = 4;  //minimum height for children
        int max_w  = 20; //maximum width for children
        int max_h  = 20; //maximum height for children

        int min_room_gen_pass_pct = 10; //minumum chance (percent) for room generation to succeed.
    };
    ////////////////////////////////////////////////////////////////////////////
    //! Axis aligned rectangular spacial division.
    ////////////////////////////////////////////////////////////////////////////
    struct region {
        bool is_leaf() const {
            return first == 0;
        }

        bool has_data() const {
            return is_leaf() && second != 0;
        }

        void split(size_t first, size_t second) {
            BK_ASSERT(is_leaf() && !has_data());
            this->first = first;
            this->second = second;
        }

        void set_data(size_t data) {
            BK_ASSERT(is_leaf() && !has_data());
            second = data;
        }

        size_t get_data() const {
            BK_ASSERT(is_leaf() && has_data());
            return second;
        }

        rect_t rect;   //! bounds for the subregion
        size_t first;  //! index of the first child region; 0 if a leaf
        size_t second; //! index of the first child region; 0 if a leaf and empty;
    };

    explicit bsp_layout(params p = params {})
      : params_ {p}
      , cat_grid_ {p.width, p.height}
      , id_grid_ {p.width, p.height}
    {
    }

    void split_region(random_t& random, region& r) {
        BK_ASSERT(r.is_leaf() && !r.has_data());

        auto const& p = params_;
        auto const  w = r.rect.width();
        auto const  h = r.rect.height();

        auto const min_w = p.min_w * 2;
        auto const min_h = p.min_h * 2;

        if ((w < min_w) && (h < min_h)) {
            //too small to split
            return;
        } else if ((w <= p.max_w) && (h <= p.max_h)
                && (std::uniform_int_distribution<> {0, 99}(random) < 50)
        ) {
            //cancel split some percent of the time if the region is smaller
            //than the max size
            return;
        }

        bool const is_vertical = (w >= min_w) && w >= h;
        auto const leeway = is_vertical ? (w - min_w) : (h - min_h);
        auto const delta = leeway ? std::uniform_int_distribution<> {0, leeway}(random) : 0;

        //calculate the two child nodes
        region r0 = r;
        region r1 = r;

        if (is_vertical) {
            r0.rect.right = r.rect.left + delta + params_.min_w;
            r1.rect.left  = r0.rect.right;
        } else {
            r0.rect.bottom = r.rect.top + delta + params_.min_h;
            r1.rect.top    = r0.rect.bottom;
        }

        //set the parent's children
        r.split(regions_root_.size() + 0, regions_root_.size() + 1);

        regions_root_.push_back(r0);
        regions_root_.push_back(r1);

    }

    rect_t generate_room(random_t& random, region const& r) {
        auto const& rect = r.rect;

        auto const w = rect.width();
        auto const h = rect.height();

        auto const leeway_x = (w - params_.min_w) / 2;
        auto const leeway_y = (h - params_.min_h) / 2;

        BK_ASSERT(leeway_x >= 0);
        BK_ASSERT(leeway_y >= 0);

        using dist_t = std::uniform_int_distribution<>;

        auto const left   = rect.left   + dist_t {0, leeway_x}(random);
        auto const top    = rect.top    + dist_t {0, leeway_y}(random);
        auto const right  = rect.right  - dist_t {0, leeway_x}(random);
        auto const bottom = rect.bottom - dist_t {0, leeway_y}(random);

        return {left, top, right, bottom};
    }

    ////////////////////////////////////////////////////////////////////////////
    //! for a given region r, return true if a room should not be generated or
    //! false otherwise.
    ////////////////////////////////////////////////////////////////////////////
    bool skip_generate_room(random_t& random, region const& r) const {
        auto const& rect = r.rect;

        auto const a     = rect.width() * rect.height(); //area
        auto const min_a = params_.min_h * params_.min_w; //min area

        BK_ASSERT(min_a > 0);
        BK_ASSERT(a >= min_a);

        //the smaller the region the higher the chance of rejection
        auto const ratio = static_cast<float>(min_a) / static_cast<float>(a);
        auto const chance = static_cast<int>(ratio * 100.0f) - params_.min_room_gen_pass_pct;
        auto const roll = roll_100(random);

        std::cout << "chance = " << chance << " roll = " << roll << " => " << (chance > roll ? "SKIP" : "GENERATE") << std::endl;

        return chance > roll;
    }

    void generate_rooms(random_t& random) {
        for_each_where(
            regions_root_
          , [](region const& r) {
                return r.is_leaf();
            }
          , [&](region& r) {
                if (skip_generate_room(random, r)) {
                    return;
                }

                auto const room_rect = generate_room(random, r);
                rooms_.push_back(room_rect);
                r.set_data(rooms_.size()); //index + 1
            }
        );
    }

    void clear() {
        regions_root_.clear();
        rooms_.clear();
        connections_.clear();
        cat_grid_.clear();
    }

    //! randomly generate a rect split into sub regions
    void generate(random_t& random) {
        clear();
        
        //reserve the absolute max number of elements possible
        regions_root_.reserve(params_.width / params_.min_w *  params_.height / params_.min_h);

        regions_root_.push_back(
            region {rect_t {0, 0, params_.width, params_.height}, 0, 0}
        );

        //dive bredth first until no nodes are left to split
        for (size_t i = 0; i < regions_root_.size(); ++i) {
            split_region(random, regions_root_[i]);
        }

        generate_rooms(random);

        room_id id = 1;
        for (auto const& room : rooms_) {
            room_to_grid(room, id++);
        }

        connect2(random, 0);

        generate_entrance(random);
        generate_exit(random);
    }

    //! rasterize a room to the final grid
    void room_to_grid(rect_t const& room, room_id id) {
        auto const is_left   = [&](grid_position_t const p) { return p.x == room.left; };
        auto const is_top    = [&](grid_position_t const p) { return p.y == room.top; };
        auto const is_right  = [&](grid_position_t const p) { return p.x == room.right - 1; };
        auto const is_bottom = [&](grid_position_t const p) { return p.y == room.bottom - 1; };

        auto const is_wall_or_door = [&](grid_position_t const p) {
            auto const value = cat_grid_.at_or_default(p.x, p.y);
            return value == tile_category::wall || value == tile_category::door;
        };

        auto const omit_top = [&](grid_position_t const p) {
            return is_wall_or_door(relative_pos(direction::nw, p))
                && is_wall_or_door(relative_pos(direction::n,  p))
                && is_wall_or_door(relative_pos(direction::ne, p));
        };

        auto const omit_bottom = [&](grid_position_t const p) {
            return is_wall_or_door(relative_pos(direction::sw, p))
                && is_wall_or_door(relative_pos(direction::s,  p))
                && is_wall_or_door(relative_pos(direction::se, p));
        };

        auto const omit_left = [&](grid_position_t const p) {
            return is_wall_or_door(relative_pos(direction::nw, p))
                && is_wall_or_door(relative_pos(direction::w,  p))
                && is_wall_or_door(relative_pos(direction::sw, p));
        };

        auto const omit_right = [&](grid_position_t const p) {
            return is_wall_or_door(relative_pos(direction::ne, p))
                && is_wall_or_door(relative_pos(direction::e,  p))
                && is_wall_or_door(relative_pos(direction::se, p));
        };

        for_each_xy(room, [&](grid_position_t const p) {
            auto const is_wall = (is_left(p)   && !omit_left(p))
                              || (is_top(p)    && !omit_top(p))
                              || (is_right(p)  && !omit_right(p))
                              || (is_bottom(p) && !omit_bottom(p));

            auto& value = cat_grid_.at(p.x, p.y);

            if (is_wall) {
                if (value == tile_category::corridor) {
                    value = tile_category::door;
                } else {
                    value = tile_category::wall;
                }
            } else {
                value = tile_category::floor;
            }

            id_grid_.at(p.x, p.y) = id;
        });
    }

    //void line_to_grid(line_t line) {
    //    if (line.x0 > line.x1) std::swap(line.x0, line.x1);
    //    if (line.y0 > line.y1) std::swap(line.y0, line.y1);

    //    auto const dx = line.x1 - line.x0;
    //    auto const dy = line.y1 - line.y0;

    //    if (dx == 0) {
    //        //vertical
    //        auto const x = line.x0;
    //        for (auto y = line.y0; y < line.y1; ++y) {
    //            cat_grid_.at(x, y) = tile_category::corridor;
    //        }
    //    } else if (dy == 0) {
    //        //horizonal
    //        auto const y = line.y0;
    //        for (auto x = line.x0; x < line.x1; ++x) {
    //            cat_grid_.at(x, y) = tile_category::corridor;
    //        }
    //    } else if (dx && dy) {
    //        auto x = line.x0;
    //        auto y = line.y0;

    //        for (auto yi = 0; yi < dy; ++yi) {
    //            cat_grid_.at(x, y++) = tile_category::corridor;
    //        }

    //        for (auto xi = 0; xi < dx; ++xi) {
    //            cat_grid_.at(x++, y) = tile_category::corridor;
    //        }
    //    }

    //}

    //void connect_regions(random_t& random, region const& r0, region const& r1) {
    //    auto const& rect0 = r0.rect;
    //    auto const& rect1 = r1.rect;
    //    
    //    auto const x0 = rect0.left + rect0.width()  / 2;
    //    auto const y0 = rect0.top  + rect0.height() / 2;

    //    auto const x1 = rect1.left + rect1.width()  / 2;
    //    auto const y1 = rect1.top  + rect1.height() / 2;

    //    line_to_grid(line_t {x0, y0, x1, y1});
    //}

    //void connect(random_t& random, region& r) {
    //    if (r.is_leaf()) {
    //        return;            
    //    }

    //    auto& r0 = regions_root_[r.first];
    //    auto& r1 = regions_root_[r.second];

    //    if (!r0.is_leaf()) {
    //        connect(random, r0);
    //    }

    //    if (!r1.is_leaf()) {
    //        connect(random, r1);
    //    }

    //    //if (r0.is_leaf() && r1.is_leaf()) {
    //        connect_regions(random, r0, r1);
    //    //}
    //}

    void tunnel(random_t& random, rect_t const from, rect_t const to) {
        static direction const dirs[] = {
            direction::n, direction::e, direction::s, direction::w
        };

        grid_position_t          p = from.center();
        grid_position_t const dest = to.center();

        auto const can_move = [&](direction const dir) -> bool {
            auto const q = relative_pos(dir, p);
            auto const value = cat_grid_.at_or(q, tile_category::invalid);

            if (value == tile_category::invalid) {
                return false;
            } else if (value != tile_category::wall) {
                return true;
            } else if (cat_grid_.at(p) == tile_category::door) {
                return false;
            }

            static auto const inv = tile_category::invalid;

            auto const n = cat_grid_.at_or(relative_pos(direction::n, q), inv);
            auto const w = cat_grid_.at_or(relative_pos(direction::w, q), inv);
            auto const e = cat_grid_.at_or(relative_pos(direction::e, q), inv);
            auto const s = cat_grid_.at_or(relative_pos(direction::s, q), inv);

            static auto const wall = tile_category::wall;

            switch (dir) {
            case yama::direction::n:
                return n != wall && w == wall && e == wall;
            case yama::direction::s:
                return s != wall && w == wall && e == wall;
            case yama::direction::w:
                return w != wall && n == wall && s == wall;
            case yama::direction::e:
                return e != wall && n == wall && s == wall;
            default:
                BK_ASSERT(false);
                break;
            }

            return false;
        };

        auto const get_dir = [&] {
            auto const v = dest - p;
            double const x = v.x;
            double const y = v.y;

            std::discrete_distribution<> dist({
                (y > 0 ? y / 10.0 : -y)
              , (x > 0 ? x : -x / 10.0)
              , (y > 0 ? y : -y / 10.0)
              , (x > 0 ? x / 10.0 : -x)
            });

            return dist(random);
        };

        auto const write_path = [&] {
            auto& value = cat_grid_.at(p);

            if (value == tile_category::wall || value == tile_category::door) {
                value = tile_category::door;
            } else if (value != tile_category::floor) {
                value = tile_category::corridor;
            }
        };

        auto const find_dir = [&](size_t const base) {
            for (size_t i = 0; i < 4; ++i) {
                auto const index = (base + i) % 4;

                if (can_move(dirs[index])) {
                    return dirs[index];
                }
            }

            BK_ASSERT(false);

            return dirs[0];
        };

        int count = 0;

        while (!to.contains(p.x, p.y)) {
            if (++count == 1000) {
                std::cout << "FAIL";
                break;
            }

            auto const base = get_dir();

            for (int i = 0; i < 5; ++i) {
                auto const dir = find_dir(base);

                p = relative_pos(dir, p);
                write_path();
            }
        }

    }

    rect_t get_room(region const& r) const {
        auto const i = r.get_data();
        BK_ASSERT(i > 0);
        return rooms_[i - 1];
    }

    size_t connect2(random_t& random, size_t const i) {
        auto const& region = regions_root_[i];

        if (region.is_leaf()) {
            if (region.has_data()) {
                return region.get_data();
            } else {
                return 0;
            }
        }

        auto const& first  = connect2(random, region.first);
        auto const& second = connect2(random, region.second);

        if (first && second) {
            std::cout << "connect " << first << " -> " << second << std::endl;

            tunnel(random, rooms_[first - 1], rooms_[second - 1]);
            return random_uniform(random, 0, 1) ? first : second;
        } else if (first) {
            return first;
        } else if (second) {
            return second;
        } else {
            return 0;
        }

        return 0;
    }

    grid_position_t get_random_rect_pos(random_t& random, rect_t rect) const {
        auto const w = rect.width();
        auto const h = rect.height();

        auto x = rect.left + random_uniform(random, 0, w-1);
        auto y = rect.top + random_uniform(random, 0, h-1);

        BK_ASSERT(rect.contains(x, y));

        while (cat_grid_.at(x, y) != tile_category::floor) {
            if (rect.is_left(x, y)) ++x;
            else if (rect.is_right(x, y)) --x;
            else if (rect.is_top(x, y)) ++y;
            else if (rect.is_bottom(x, y)) --y;
            else BK_ASSERT(false);
        }

        return {x, y};
    }

    void generate_entrance(random_t& random) {
        auto const& room = rooms_[0];
        auto const p = get_random_rect_pos(random, room);
        cat_grid_.at(p.x, p.y) = tile_category::stair;
    }

    void generate_exit(random_t& random) {
        auto const& room = rooms_[rooms_.size() - 1];
        auto const p = get_random_rect_pos(random, room);
        cat_grid_.at(p.x, p.y) = tile_category::stair;
    }

    params params_;

    std::vector<region> regions_root_;
    std::vector<rect_t> rooms_;
    std::vector<line_t> connections_;

    grid<tile_category> cat_grid_;
    grid<room_id>       id_grid_;

    grid_position_t entrace_pos_;
    grid_position_t exit_pos_;

    int count_ = 0;
};

class map {
public:
    explicit map(random_t& random)
      : category_ {64, 64}
      , room_id_ {64, 64}
    {
        bsp_layout::params params;
        params.height = 64;
        params.width = 64;

        bsp_layout layout {params};
        layout.generate(random);
        category_ = layout.cat_grid_;

        regions_ = layout.regions_root_;
    }

    void render_tile(renderer& renderer, int x, int y, tile_category cat) const {
        switch (cat) {
        case yama::tile_category::empty :
            renderer.set_color(0, 0, 0);
            break;
        case yama::tile_category::wall :
            renderer.set_color(200, 200, 200);
            break;
        case yama::tile_category::floor :
            renderer.set_color(150, 100, 100);
            break;
        case yama::tile_category::door :
            renderer.set_color(200, 100, 100);
            break;
        case yama::tile_category::corridor :
            renderer.set_color(100, 100, 100);
            break;
        case yama::tile_category::stair :
            renderer.set_color(255, 255, 0);
            break;
        default:
            break;
        }

        renderer.fill_rect(x*16, y*16, 16, 16);
    }

    void render(renderer& renderer) const {
        for_each_xy(category_, [&](int x, int y, tile_category cat) {
            render_tile(renderer, x, y, cat);
        });

        renderer.set_color(255, 255, 255);
        std::for_each(regions_.begin(), regions_.end(), [&](bsp_layout::region const& r) {
            if (!r.is_leaf()) return;
            renderer.draw_rect(r.rect.left*16, r.rect.top*16, r.rect.width()*16, r.rect.height()*16);
        });
    }


    room_id room_id_at(grid_position_t const p) const {
    }

    void set_room_id_at(grid_position_t p) {
    }
private:
    grid<tile_category> category_;
    grid<room_id> room_id_;
    std::vector<bsp_layout::region> regions_;
};

class level {
public:
    explicit level(random_t& random)
      : map_ {random}
    {
    }

    void render(renderer& renderer) {
        map_.render(renderer);
    }
private:
    map map_;
};

class world {
public:
    explicit world(random_t& random)
      : levels_ {random}
    {
    }

    void render(renderer& renderer) {
        levels_.render(renderer);

        renderer.set_color(100, 100, 200);
        renderer.fill_rect(player_x_*16, player_y_*16, 16, 16);
    }

    void move_player(int dx, int dy) {
        player_x_ += dx;
        player_y_ += dy;
    }
private:
    level levels_;

    int player_x_ = 0;
    int player_y_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
//! The game engine; divorced from system specifics.
////////////////////////////////////////////////////////////////////////////////
class engine {
public:
    engine()
      : random_ {1002}
      , client_ {std::bind(&engine::on_command, this, std::placeholders::_1)}
      , renderer_ {client_.handle()}
      , world_ {random_}
    {
    }

    void update() {
    }

    void render() {
        renderer_.set_color(1.0f, 0.0f, 0.0f);
        renderer_.clear();

        world_.render(renderer_);

        renderer_.present();
    }

    void run() {
        while (client_) {
            client_.do_events();
            render();
        }
    }

    void on_command(command_type cmd) {
        switch (cmd) {
        case yama::command_type::none:
            break;
        case yama::command_type::move_nw:
            break;
        case yama::command_type::move_n:
            world_.move_player(0, -1);
            break;
        case yama::command_type::move_ne:
            break;
        case yama::command_type::move_w:
            world_.move_player(-1, 0);
            break;
        case yama::command_type::move_here:
            break;
        case yama::command_type::move_e:
            world_.move_player(1, 0);
            break;
        case yama::command_type::move_sw:
            break;
        case yama::command_type::move_s:
            world_.move_player(0, 1);
            break;
        case yama::command_type::move_se:
            break;
        case yama::command_type::cancel:
            break;
        default:
            break;
        }
    }

    void on_motion(int dx, int dy) {
    }

    void on_move_to(int x, int y) {
    }
private:
    random_t random_;
    client   client_;
    renderer renderer_;
    world    world_;
};

} //namespace yama

struct entity {
};

struct item {
};

struct player {
};

////////////////////////////////////////////////////////////////////////////////
//! Entry point for the SDL application.
////////////////////////////////////////////////////////////////////////////////
int SDL_main(int argc, char* argv[]) {
    yama::engine e;

    e.run();

    return 0;
}
