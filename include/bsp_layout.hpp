#pragma once

#include <memory>

#include "types.hpp"
#include "math.hpp"
#include "map.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! BSP tree based map layout generator.
//!
//! pimpl based
////////////////////////////////////////////////////////////////////////////////
class bsp_layout {
public:
    using on_create_room_t = std::function<
        bool               //!< true if a room was generated; false otherwise.
        (random_t&  random //!< the random number generator to use.
       , yama::map& map    //!< the map to write to.
       , rect_t     bounds //!< the bounds for the room.
    )>;

    using on_connect_rooms_t = std::function<
        bool               //!< true if a connection was made; false otherwise.
        (random_t&  random //!< the random number generator to use.
       , yama::map& map    //!< the map to write to.
       , rect_t     first  //!< the bounds for the first room.
       , rect_t     second //!< the bounds for the second room.
    )>;

    ////////////////////////////////////////////////////////////////////////////
    //! BSP layout generation parameters.
    ////////////////////////////////////////////////////////////////////////////
    struct params_t {
        params_t() {}

        positive_interval room_w_range {4, 25}; //!< The min/max range for room width.
        positive_interval room_h_range {4, 25}; //!< The min/max range for room height.       

        closed_range<int, 100> room_size_weight {0};
        closed_range<int, 100> room_size_variance {0};

        positive<int> border_size {0}; //!< The minimum spacing between regions.

        //! The The min/max range for region width.
        positive_interval region_w_range {room_w_range.lower + border_size, room_w_range.upper};
        //! The The min/max range for region height.
        positive_interval region_h_range {room_h_range.lower + border_size, room_h_range.upper};

        positive_interval corridor_segment_length_range {2, 10};

        strict_percentage<int> room_generation_chance {10};
        strict_percentage<int> region_split_chance {25};

        //! The aspect ratio threshhold to decide how to split a rect.
        aspect_ratio<float> split_aspect {5.0f / 4.0f};

        //! The aspect ratio threshhold to force a region to be split.
        aspect_ratio<float> split_limit_aspect {16.0f / 10.0f};

        positive<float> corridor_randomness {0.25f};
    };

    explicit bsp_layout(params_t p = params_t {});
    ~bsp_layout();

    void generate(
        random_t&           random
      , yama::map&          map
      , on_create_room_t&   on_create_room
      , on_connect_rooms_t& on_connect_rooms
    );
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace yama
