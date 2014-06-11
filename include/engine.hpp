#pragma once

#include "commands.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! The game engine; divorced from system specifics.
////////////////////////////////////////////////////////////////////////////////
class engine {
public:
    engine();

    ~engine();

    void update();

    void render();

    void run();

    void on_command(command_type cmd);

    void on_motion(int dx, int dy);

    void on_move_to(int x, int y);
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace yama
