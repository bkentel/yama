#pragma once

#include "random.hpp"
#include "renderer.hpp"
#include "map.hpp"

#include "bsp_layout.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! One game level.
////////////////////////////////////////////////////////////////////////////////
class level {
public:
    explicit level(random_t& random, map_size width, map_size height)
      : map_ {10, 10} //TODO fix
    {
        bsp_layout::params_t p{};
        p.room_size_weight = 100;

        bsp_layout layout {p};
        map_ = layout.generate(random);
        regions_ = layout.get_regions();
    }

    void render(renderer& r) {
        auto const w = map_.width();
        auto const h = map_.height();

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                auto const cat = map_.get<map_property::category>(x, y);

                using category = yama::tile_category;

                switch (cat) {
                case category::empty:    r.set_color(0, 0, 0); break;
                case category::wall:     r.set_color(100, 100, 100); break;
                case category::floor:    r.set_color(200, 200, 200); break;
                case category::door:     r.set_color(0, 0, 200); break;
                case category::corridor: r.set_color(0, 100, 0); break;
                case category::stair:    r.set_color(255, 0, 0); break;
                case category::invalid:  r.set_color(100, 100, 200); break;
                default:                 r.set_color(100, 100, 200); break;
                }

                r.fill_rect(x*16, y*16, 16, 16);
            }
        }

        r.set_color(255, 0, 0);
        for (auto const& region : regions_) {
            r.draw_rect(region.left*16, region.top*16, region.width()*16, region.height()*16);
        }
    }

    int width()  const { return map_.width(); }
    int height() const { return map_.height(); }
private:
    map map_;
    std::vector<rect_t> regions_;
};

} //namespace yama
