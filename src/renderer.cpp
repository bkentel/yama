#include "pch.hpp"
#include "renderer.hpp"

using yama::renderer;
using yama::window_handle;

class renderer::impl_t {
public:
    using renderer_ptr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

    static renderer_ptr create_renderer(window_handle window) {
        auto result = renderer_ptr {
            SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED), &SDL_DestroyRenderer
        };

        BK_ASSERT(result.get() != nullptr);

        return result;
    }

    impl_t(window_handle window)
      : renderer_ {create_renderer(window)}
    {
    }

    void set_color(float r, float g, float b, float a = 1.0f) {
        set_color(
            static_cast<uint8_t>(r * 255)
          , static_cast<uint8_t>(g * 255)
          , static_cast<uint8_t>(b * 255)
          , static_cast<uint8_t>(a * 255)
        );
    }

    void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        SDL_SetRenderDrawColor(renderer_.get(), r, g, b, a);
    }

    template <typename T>
    void fill_rect(T x, T y, T w, T h) {
        SDL_Rect const rect = {
            static_cast<int>(x)
          , static_cast<int>(y)
          , static_cast<int>(w)
          , static_cast<int>(h)
        };

        SDL_RenderFillRect(renderer_.get(), &rect);
    }

    template <typename T>
    void draw_rect(T x, T y, T w, T h) {
        SDL_Rect const rect = {
            static_cast<int>(x)
          , static_cast<int>(y)
          , static_cast<int>(w)
          , static_cast<int>(h)
        };

        SDL_RenderDrawRect(renderer_.get(), &rect);
    }


    void clear() {
        SDL_RenderClear(renderer_.get());
    }

    void present() {
        SDL_RenderPresent(renderer_.get());
    }

private:
    renderer_ptr renderer_;
};


renderer::renderer(window_handle window)
  : impl_ {std::make_unique<impl_t>(window)}
{
}

renderer::~renderer()
{
}

void renderer::set_color(float r, float g, float b, float a) {
    impl_->set_color(r, g, b, a);
}

void renderer::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    impl_->set_color(r, g, b, a);
}

void renderer::fill_rect(int x, int y, int w, int h) {
    impl_->fill_rect(x, y, w, h);
}

void renderer::draw_rect(int x, int y, int w, int h) {
    impl_->draw_rect(x, y, w, h);
}

void renderer::clear() {
    impl_->clear();
}

void renderer::present() {
    impl_->present();
}

