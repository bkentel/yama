#include "pch.hpp"
#include "grid.hpp"

#include <catch/catch.hpp>

TEST_CASE("grid can be initialized and cleared", "[grid]") {
    using grid_t = yama::grid<int>;

    constexpr int w     = 10;
    constexpr int h     = 20;
    constexpr int value = 0xBEEF;

    grid_t grid {w, h, value};

    auto const check_values = [](grid_t& grid, int const value) {
        yama::for_each_xy(grid, [&](int x, int y, int val) {
            yama::grid_position_t const p {x, y};

            REQUIRE(grid(p.x, p.y) == val);
            REQUIRE(grid[p] == val);
            REQUIRE(val == value);
        });
    };

    SECTION("dimensions") {
        REQUIRE(grid.width()  == w);
        REQUIRE(grid.height() == h);
    }

    SECTION("default value") {
        grid_t grid {w, h};
        check_values(grid, int {});
    }

    SECTION("chosen default value") {
        check_values(grid, value);
    }

    SECTION("clear to value") {
        constexpr int new_value = 0xDEAD;
        grid.clear(new_value);
        check_values(grid, new_value);
    }

    SECTION("clear to default") {
        grid.clear();
        check_values(grid, int {});
    }
}
