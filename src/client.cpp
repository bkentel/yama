#include "pch.hpp"
#include "client.hpp"

using yama::client;
using yama::window_handle;

class yama::client::impl_t {
public:
    using window_ptr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

    static window_ptr create_window() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            BK_ABORT_TODO();
        }

        auto result = window_ptr { SDL_CreateWindow(
            "test"
           , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
           , 1024, 768
           , SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE //| SDL_WINDOW_OPENGL
        ), &SDL_DestroyWindow };

        if (!result) {
            BK_ABORT_TODO();
        }

        return result;
    }

    impl_t(command_sink_t command_sink)
      : window_ {create_window()}
      , running_ {false}
      , command_sink_ {command_sink}
    {
        running_ = true;
    }

    ~impl_t() {
        SDL_Quit();
    }

    window_handle handle() const {
        return window_.get();
    }

    void do_events() {
        SDL_Event event {};
    
        while (SDL_PollEvent(&event)) {
            dispatch_event_(event);
        }
    }

    void shutdown() {
        BK_ASSERT(running_ == true);
        running_ = false;        
    }

    explicit operator bool() const {
        return running_;
    }
private:
    window_ptr     window_;
    bool           running_;
    command_sink_t command_sink_;

    void dispatch_event_(SDL_Event const& event) {
        switch (event.type) {
        case SDL_WINDOWEVENT :
            do_window_events_(event.window);
            break;
        case SDL_MOUSEMOTION :
        case SDL_MOUSEBUTTONDOWN :
        case SDL_MOUSEBUTTONUP :
            break;
        case SDL_KEYDOWN :
        case SDL_KEYUP :
            do_keyboard_events_(event.key);
            break;
        default :
            break;
        }
    }

    void do_window_events_(SDL_WindowEvent const& event) {
        switch (event.event) {
        case SDL_WINDOWEVENT_CLOSE :
            shutdown();
            break;
        default :
            break;
        }
    }

    void do_keyboard_events_(SDL_KeyboardEvent const& event) {
        if (event.type != SDL_KEYDOWN) {
            return;
        }

        auto const& sym = event.keysym.sym;
        auto const& mod = event.keysym.mod;

        switch (sym) {
        case SDLK_ESCAPE :
            if (mod == 0) {
                command_sink_(command_type::cancel);
            }
            break;
        case SDLK_UP :
            if (mod == 0) {
                command_sink_(command_type::move_n);
            }
            break;
        case SDLK_DOWN :
            if (mod == 0) {
                command_sink_(command_type::move_s);
            }
            break;
        case SDLK_LEFT :
            if (mod == 0) {
                command_sink_(command_type::move_w);
            }
            break;
        case SDLK_RIGHT :
            if (mod == 0) {
                command_sink_(command_type::move_e);
            }
            break;
        }
    }
};

yama::client::client(command_sink_t command_sink)
  : impl_ {std::make_unique<impl_t>(command_sink)}
{
}

yama::client::~client() {
}

window_handle yama::client::handle() const {
    return impl_->handle();
}

void yama::client::do_events() {
    impl_->do_events();
}

void client::shutdown() {
    impl_->shutdown();
}

client::operator bool() const {
    return static_cast<bool>(*impl_);
}
