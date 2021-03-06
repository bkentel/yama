#include "pch.hpp"

#include <catch/catch.hpp>

#include "detail/bsp_layout_impl.hpp"

using bsp_layout_impl = yama::detail::bsp_layout_impl;
using yama::rect_t;

//TEST_CASE("bsp_layout_regions", "[bsp_layout]") {
//    yama::random_t random {1002};
//
//    for (int i = 0; i < 100; ++i) {
//        std::cout << yama::random_percent(random) << " ";
//    }
//
//    bsp_layout_impl bsp {yama::bsp_layout::params_t {}};
//
////for(int i = 0; i < 100; ++i) {
//    auto const test_map = bsp.generate(random);
//
//    auto const w = test_map.width();
//    auto const h = test_map.height();
//
//    std::cout << std::endl;
//    for (int y = 0; y < h; ++y) {
//        for (int x = 0; x < w; ++x) {
//            auto const value = test_map.get<yama::map::property::category>(x, y);
//
//            switch (value) {
//            case yama::tile_category::empty :    std::cout << ' '; break;
//            case yama::tile_category::corridor : std::cout << 'X'; break;
//            case yama::tile_category::floor :    std::cout << '.'; break;
//            case yama::tile_category::wall :     std::cout << '#'; break;
//            case yama::tile_category::door :     std::cout << '0'; break;
//            default:
//                std::cout << '?'; break;
//                break;
//            }
//        }
//        std::cout << std::endl;
//    }
//
//    bsp.clear();
////}
//}
