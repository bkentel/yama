#pragma once

#include "engine.hpp"
#include "random.hpp"
#include "client.hpp"
#include "renderer.hpp"
#include "world.hpp"

namespace yama {
namespace detail {

class engine_impl {
public:
    engine_impl()
      : client_ {std::bind(&engine_impl::on_command, this, std::placeholders::_1)}
      , renderer_ {client_.handle()}
      , world_ {random_substantive_}
    {
    }

    void update() {
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
private:
    random_t random_substantive_ {1002};
    random_t random_nominal_     {1002};

    client   client_;
    renderer renderer_;
    world    world_;
};

} //namespace detail
} //namespace yama
