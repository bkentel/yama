#pragma once

#include "random.hpp"
#include "renderer.hpp"
#include "level.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! The entire game "world".
////////////////////////////////////////////////////////////////////////////////
class world {
public:
    explicit world(random_t& random)
      : levels_ {random, 100, 100}
    {
    }

    void render(renderer& r) {
        levels_.render(r);

        r.set_color(100, 100, 200);
        r.fill_rect(player_x_*16, player_y_*16, 16, 16);
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

} //namespace yama
