#include "pch.hpp"
#include "assert.hpp"
#include "types.hpp"
#include "client.hpp"
#include "renderer.hpp"
#include "engine.hpp"

#include "checked_value.hpp"

namespace yama {

//class map {
//public:
//    explicit map(random_t& random)
//      : category_ {64, 64}
//      , room_id_ {64, 64}
//    {
//        bsp_layout::params params;
//        params.height = 64;
//        params.width = 64;
//
//        bsp_layout layout {params};
//        layout.generate(random);
//        category_ = layout.cat_grid_;
//
//        regions_ = layout.regions_root_;
//    }
//
//    void render_tile(renderer& renderer, int x, int y, tile_category cat) const {
//        switch (cat) {
//        case yama::tile_category::empty :
//            renderer.set_color(0, 0, 0);
//            break;
//        case yama::tile_category::wall :
//            renderer.set_color(200, 200, 200);
//            break;
//        case yama::tile_category::floor :
//            renderer.set_color(150, 100, 100);
//            break;
//        case yama::tile_category::door :
//            renderer.set_color(200, 100, 100);
//            break;
//        case yama::tile_category::corridor :
//            renderer.set_color(100, 100, 100);
//            break;
//        case yama::tile_category::stair :
//            renderer.set_color(255, 255, 0);
//            break;
//        default:
//            break;
//        }
//
//        renderer.fill_rect(x*16, y*16, 16, 16);
//    }
//
//    void render(renderer& renderer) const {
//        for_each_xy(category_, [&](int x, int y, tile_category cat) {
//            render_tile(renderer, x, y, cat);
//        });
//
//        renderer.set_color(255, 255, 255);
//        std::for_each(regions_.begin(), regions_.end(), [&](bsp_layout::region const& r) {
//            if (!r.is_leaf()) return;
//            renderer.draw_rect(r.rect.left*16, r.rect.top*16, r.rect.width()*16, r.rect.height()*16);
//        });
//    }
//
//
//    room_id room_id_at(grid_position_t const p) const {
//    }
//
//    void set_room_id_at(grid_position_t p) {
//    }
//private:
//    grid<tile_category> category_;
//    grid<room_id> room_id_;
//    std::vector<bsp_layout::region> regions_;
//};






} //namespace yama

////////////////////////////////////////////////////////////////////////////////
//! Entry point for the SDL application.
////////////////////////////////////////////////////////////////////////////////


int SDL_main(int argc, char* argv[]) {
    using value_t = yama::strict_percentage<>;

    value_t x = 100;
    value_t y = -100;

    std::cout << x << " " << y << std::endl;

    yama::engine e;

    e.run();

    return 0;
}
