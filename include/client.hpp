#pragma once

#include "config.hpp"
#include "commands.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! System level "application" management.
////////////////////////////////////////////////////////////////////////////////
class client {
public:
    using command_sink_t = std::function<void (command_type)>;

    explicit client(command_sink_t command_sink);
    ~client();

    window_handle handle() const;

    void do_events();

    void shutdown();

    explicit operator bool() const;
private:
    client(client& other) = delete;
    client& operator=(client& other) = delete;

    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace yama
