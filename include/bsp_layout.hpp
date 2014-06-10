#pragma once

#include <memory>

#include "types.hpp"
#include "map.hpp"

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! BSP tree based map layout generator.
//
//! pimpl based
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
struct closed_integral_interval {
    closed_integral_interval(T const Lower, T const Upper)
      : lower {Lower}
      , upper {Upper}
    {
        BK_ASSERT(lower <= upper);
    }

    T lower;
    T upper;

    bool operator<(T const value) const {
        return value < lower;
    }

    bool operator>(T const value) const {
        return value > upper;
    }

    bool operator<=(T const value) const {
        return value <= lower;
    }

    bool operator>=(T const value) const {
        return value >= upper;
    }

    bool contains(T const value) const {
        return value >= lower && value <= upper;
    }

    T clamp(T const value) const {
        return value < lower ? lower : value > upper ? upper : value;
    }

};

struct restriction_percentage {
    template <typename T>
    static bool check(T const value) {
        constexpr auto lower = T{0};
        constexpr auto upper = T{100};

        return value >= lower && value <= upper;
    }

    template <typename T>
    static T apply(T const value) {
        constexpr auto lower = T{0};
        constexpr auto upper = T{100};

        return value > upper ? upper : value < lower ? lower : value;
    }
};

//struct restriction_positive {
//    template <typename T>
//    static bool check(T const value) {
//        constexpr auto lower = T{0};
//        return value >= lower;
//    }
//
//    template <typename T>
//    static T apply(T const value) {
//        constexpr auto lower = T{0};
//        return value < lower ? lower : value;
//    }
//};

template <int Min>
struct restriction_minimum {
    template <typename T>
    static bool check(T const value) {
        constexpr auto lower = T{Min};
        return value >= lower;
    }

    template <typename T>
    static T apply(T const value) {
        constexpr auto lower = T{Min};
        return value < lower ? lower : value;
    }
};

struct restriction_aspect_ratio {
    template <typename T>
    static bool check(T const value) {
        constexpr auto lower = T{1};
        return value >= lower;
    }

    template <typename T>
    static T apply(T const value) {
        constexpr auto lower = T{1};
        return value < lower ? lower : value;
    }
};

template <typename T, typename Restriction>
class restricted_value {
public:
    restricted_value(T const value = T{0})
      : value_ { Restriction::apply(value) }
    {
        BK_ASSERT(Restriction::check(value));
    }

    bool operator<(T const rhs)  const { return value_ <  rhs; }
    bool operator<=(T const rhs) const { return value_ <= rhs; }
    bool operator>(T const rhs)  const { return value_ >  rhs; }
    bool operator>=(T const rhs) const { return value_ >= rhs; }

    restricted_value operator=(T const rhs) {
        BK_ASSERT(Restriction::check(rhs));

        value_ = Restriction::apply(rhs);
        return *this;
    }

    operator T() const { return value_; }
private:
    T value_;
};

using percentage = restricted_value<int, restriction_percentage>;

template <typename T>
using positive = restricted_value<T, restriction_minimum<0>>;
using aspect_ratio = restricted_value<float, restriction_aspect_ratio>;

class bsp_layout {
public:
    ////////////////////////////////////////////////////////////////////////////
    //! BSP layout generation parameters.
    ////////////////////////////////////////////////////////////////////////////
    struct params_t {
        using interval = closed_integral_interval<positive<int>>;
        using map_size = restricted_value<int, restriction_minimum<10>>;

        params_t() {}

        map_size map_w {150}; //<! Generated map width.
        map_size map_h {75};  //<! Generated map height.

        interval room_w_range {4, 25}; //!< The min/max range for room width.
        interval room_h_range {4, 25}; //!< The min/max range for room height.

        positive<int> border_size {0}; //!< The minimum spacing between regions.

        //! The The min/max range for region width.
        interval region_w_range {room_w_range.lower + border_size, room_w_range.upper};
        //! The The min/max range for region height.
        interval region_h_range {room_h_range.lower + border_size, room_h_range.upper};

        interval corridor_segment_length_range {2, 10};

        percentage room_generation_chance {50};
        percentage region_split_chance {25};

        aspect_ratio split_aspect {5.0f / 4.0f};
        aspect_ratio split_limit_aspect {16.0f / 10.0f};

        positive<float> corridor_randomness {0.25f};

        //int map_w = 150; //! generated map width.
        //int map_h = 75;  //! generated map height.

        int min_room_w = 4; //!< minimum generated room width
        int min_room_h = 4; //!< minimum generated room height

        int max_room_w = 25; //!< max room width
        int max_room_h = 25; //!< max room height

        int border = 0;

        int min_region_w = min_room_w + border;
        int min_region_h = min_room_h + border;

        int max_region_w = max_room_w + border;
        int max_region_h = max_room_h + border;

        //! percent [0, 100] chance to generate a room in a cell.
        int room_generation_rate = 50;

        //! the range of lengths for corridor segments
        int corridor_seg_len_min = 2;
        int corridor_seg_len_max = 10;

        //! the randomness multiplier for corridor creation
        int corridor_randomness_num = 1;
        int corridor_randomness_den = 4;

        //! the ratio that must be expceeded to prefer one dimension over another.
        float split_ratio = 5.0f / 4.0f;

        //! the ratio that if exceeded will force a split.
        float split_limit = 16.0f / 10.0f;

        int split_chance = 25;
    };

    explicit bsp_layout(params_t p = params_t {});
    ~bsp_layout();

    params_t params() const;
    void set_params(params_t p = params_t {});

    map generate(random_t& random);
private:
    class impl_t;
    std::unique_ptr<impl_t> impl_;
};

} //namespace yama
