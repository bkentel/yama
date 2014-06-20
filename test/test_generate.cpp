#include "pch.hpp"
#include <catch/catch.hpp>
#include "generate.hpp"

using namespace yama;


TEST_CASE("test distribution", "[generate]") {
     yama::random_t random {1011};

    constexpr int n = 2500000;

    auto const range = yama::closed_integral_interval<> {0, 99};
    constexpr int weight = 0;

    auto const count = range.size() + 1;

    auto result = std::vector<int>(count, 0);


    for (auto i = 0; i < n; ++i) {
        auto const value = generate::weighted_range(random, range, -100, -75);
        result[value] += 1;
    }

    auto const print = [&](int value) {
        auto const m = value / 800;
        for (int i = 0; i < m; ++i) {
            std::cout << "*";
        }
    };

    for (auto i = 0; i < count; ++i) {
        std::cout << "[" << i << "\t]";
        print(result[i]);
        std::cout << std::endl;
    }
}


TEST_CASE("generate range value", "[generate]") {
    yama::random_t random {1011};

    constexpr int n = 10000;

    constexpr int lower = 5;
    constexpr int upper = 20;
    constexpr int weight = 0;

    std::vector<int> result (upper - lower + 1, 0);

    auto const range = yama::closed_integral_interval<> {lower, upper};

    for (auto i = 0; i < n; ++i) {
        auto const value = generate::weighted_range(random, range, weight, 0);

        REQUIRE(value >= lower);
        REQUIRE(value <= upper);

        auto const index = value - lower;
        result[index] += 1;
    }

    for (auto const& i : result) {
        REQUIRE(i > 0);
    }

}

TEST_CASE("generate rects", "[generate]") {
    yama::random_t random {1011};

    constexpr int left   = -10;
    constexpr int top    = -20;
    constexpr int right  = 10;
    constexpr int bottom = 20;

    constexpr int min_w = 5;
    constexpr int min_h = 10;

    yama::rect_t const bounds {left, top, right, bottom};

    for (int i = 0; i < 100; ++i) {
        auto const r = yama::generate::uniform_bounded_rect(random, bounds, min_w, min_h);

        REQUIRE(r.width()  >= min_w);
        REQUIRE(r.height() >= min_h);

        REQUIRE(r.left   >= left);
        REQUIRE(r.top    >= top);
        REQUIRE(r.right  <= right);
        REQUIRE(r.bottom <= bottom);
    }

}

TEST_CASE("generate split_rect", "[generate]") {
    yama::random_t random {1001};

    constexpr int min_w = 4;
    constexpr int min_h = 5;

    auto const get = [&](rect_t const rect) {
        return generate::get_split_type(random, rect, min_w, min_h, 1.0f);
    };

    SECTION("too small") {
        rect_t const rect {0, 0, min_w, min_h};
        REQUIRE(get(rect) == generate::split_type::none);
    }

    SECTION("horizontal") {
        rect_t const rect {0, 0, 10, 11};
        REQUIRE(get(rect) == generate::split_type::horizontal);
    }

    SECTION("vertical") {
        rect_t const rect {0, 0, 11, 10};
        REQUIRE(get(rect) == generate::split_type::vertical);
    }

    SECTION("random") {
        rect_t const rect {0, 0, 10, 10};

        bool vertical {false};
        bool horizontal {false};
        int const limit = 100;

        for (int i = 0; i < limit; ++i) {
            switch (get(rect)) {
            case generate::split_type::none :
                FAIL("unexpected");
                break;
            case generate::split_type::vertical :
                vertical = true;
                break;
            case generate::split_type::horizontal :
                horizontal = true;
                break;
            }

            if (vertical && horizontal) {
                break;
            }
        }

        REQUIRE((vertical && horizontal));
    }
}

TEST_CASE("bsp_layout_regions split_rect", "[bsp_layout]") {
    yama::random_t random {1001};

    constexpr int w = 100;
    constexpr int h = 100;
    constexpr int min_w = 4;
    constexpr int min_h = 5;

    rect_t const rect {0, 0, w, h};

    auto const get = [&](generate::split_type const type) {
        return generate::split_rect(random, rect, type, min_w, min_h);
    };

    SECTION("horizontal") {
        auto const type = generate::split_type::horizontal;
        auto const result = get(type);

        REQUIRE(std::get<0>(result) == type);

        rect_t const first  = std::get<1>(result);
        rect_t const second = std::get<2>(result);

        REQUIRE(first.left  == rect.left);
        REQUIRE(first.right == rect.right);
        REQUIRE(first.top   == rect.top);

        REQUIRE(second.left   == second.left);
        REQUIRE(second.right  == second.right);
        REQUIRE(second.bottom == second.bottom);

        REQUIRE(first.bottom == second.top);

        REQUIRE(first.height()  >= min_h);
        REQUIRE(second.height() >= min_h);
    }

    SECTION("vertical") {
        auto const type = generate::split_type::vertical;
        auto const result = get(type);

        REQUIRE(std::get<0>(result) == type);

        rect_t const first  = std::get<1>(result);
        rect_t const second = std::get<2>(result);

        REQUIRE(first.top    == rect.top);
        REQUIRE(first.bottom == rect.bottom);
        REQUIRE(first.left   == rect.left);

        REQUIRE(second.top    == second.top);
        REQUIRE(second.bottom == second.bottom);
        REQUIRE(second.right  == second.right);

        REQUIRE(first.right == second.left);

        REQUIRE(first.width()  >= min_w);
        REQUIRE(second.width() >= min_w);
    }
}
