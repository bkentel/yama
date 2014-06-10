#include "pch.hpp"
#include "math.hpp"

#include <catch/catch.hpp>

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
