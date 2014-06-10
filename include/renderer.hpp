#pragma once

#include "config.hpp"

namespace yama {

class renderer {  
public:
    struct rect {
        float x0, y0, x1, y1;
    };

    struct color {
        float r, g, b, a;
    };

    struct texture_rect {
        float x0, y0, x1, y1;
    };

    explicit renderer(window_handle window);
    ~renderer();

    void set_color(float r, float g, float b, float a = 1.0f);
    void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    template <typename T>
    void set_color(T r, T g, T b, T a = 255) {
        set_color(
            static_cast<uint8_t>(r & 0xFF)
          , static_cast<uint8_t>(g & 0xFF)
          , static_cast<uint8_t>(b & 0xFF)
          , static_cast<uint8_t>(a & 0xFF)
        );
    }

    void fill_rect(int x, int y, int w, int h);
    void draw_rect(int x, int y, int w, int h);

    void clear();

    void present();
private:
    renderer(renderer&) = delete;
    renderer& operator=(renderer&) = delete;

    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace yama
