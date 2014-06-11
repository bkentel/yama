#include "pch.hpp"
#include "engine.hpp"
#include "detail/engine_impl.hpp"

//==============================================================================
class yama::engine::impl_t : public detail::engine_impl {
    using engine_impl::engine_impl;
};

yama::engine::engine()
  : impl_ {std::make_unique<impl_t>()}
{
}

yama::engine::~engine() {
}

void yama::engine::update() {
    impl_->update();
}

void yama::engine::render() {
    impl_->render();
}

void yama::engine::run() {
    impl_->run();
}

void yama::engine::on_command(command_type const cmd) {
    impl_->on_command(cmd);
}

void yama::engine::on_motion(int dx, int dy) {
}

void yama::engine::on_move_to(int x, int y) {
}
