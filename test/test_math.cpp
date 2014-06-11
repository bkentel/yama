#include "pch.hpp"
#include "math.hpp"

#include <catch/catch.hpp>

TEST_CASE("closed_integral_interval", "[interval][point2d]") {
    constexpr int lower = 1;
    constexpr int upper = 10;

    yama::closed_integral_interval<> const interval {lower, upper};

    REQUIRE(interval.lower == lower);
    REQUIRE(interval.upper == upper);

    REQUIRE((lower - 1) <  interval);
    REQUIRE(lower       <= interval);
    REQUIRE((upper + 1) >  interval);
    REQUIRE(upper       >= interval);

    for (int i = lower; i <= upper; ++i) {
        REQUIRE(interval.contains(i));
    }

    REQUIRE(interval.clamp(lower - 1) == lower);
    REQUIRE(interval.clamp(upper + 1) == upper);

    for (int i = lower; i <= upper; ++i) {
        REQUIRE(interval.clamp(i) == i);
    }
}

TEST_CASE("points are initialized", "[math][point2d]") {
    constexpr int x = 1;
    constexpr int y = 2;

    yama::point2d<int> point {x, y};

    REQUIRE(point.x == x);
    REQUIRE(point.y == y);

    REQUIRE(point == point);
}

TEST_CASE("rects are initialized", "[math][axis_aligned_rect]") {
    using rect_t  = yama::axis_aligned_rect<int>;
    using point_t = yama::point2d<int>;

    constexpr int left   = 1;
    constexpr int right  = 2;
    constexpr int top    = 3;
    constexpr int bottom = 4;

    rect_t rect {left, top, right, bottom};

    REQUIRE(rect == rect);

    REQUIRE(rect.left   == left);
    REQUIRE(rect.right  == right);
    REQUIRE(rect.top    == top);
    REQUIRE(rect.bottom == bottom);

    REQUIRE(rect.width()  == 1);
    REQUIRE(rect.height() == 1);
    REQUIRE(rect.area()   == 1);

    SECTION("rect border tests") {
        REQUIRE(rect.is_top(point_t {left, top}));
        REQUIRE(rect.is_left(point_t {left, top}));
        REQUIRE(rect.is_bottom(point_t {right - 1, bottom - 1}));
        REQUIRE(rect.is_right(point_t {right - 1, bottom - 1}));
    }
    
    SECTION("rect <-> point intersection") {
        REQUIRE(rect.contains(point_t {left, top}));
        REQUIRE(rect.contains(point_t {right - 1, top}));
        REQUIRE(rect.contains(point_t {right - 1, bottom - 1}));
        REQUIRE(rect.contains(point_t {left, bottom - 1}));
    }
}
