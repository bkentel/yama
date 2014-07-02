#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>

#include "types.hpp"
#include "math.hpp"
#include "generate.hpp"
#include "map.hpp"

namespace yama {

template <typename Tag, typename Type = int>
class tagged_id {
public:
    using value_type = Type;
    using tag_type = Tag;

    explicit tagged_id(value_type const Value) : value {Value} {}
    tagged_id(tagged_id const&) = default;
    tagged_id& operator=(tagged_id const&) = default;

    bool operator==(tagged_id const rhs) const { return value == rhs.value; };
    bool operator!=(tagged_id const rhs) const { return value != rhs.value; };
    bool operator<=(tagged_id const rhs) const { return value <= rhs.value; };
    bool operator>=(tagged_id const rhs) const { return value >= rhs.value; };
    bool operator< (tagged_id const rhs) const { return value <  rhs.value; };
    bool operator> (tagged_id const rhs) const { return value >  rhs.value; };

    value_type value;
};

class bsp_layout_dev {
public:
    using region_id = tagged_id<bsp_layout_dev>;
    using room_generator = std::function<bool (region_id region, rect_t bounds)>;
    using region_connector = std::function<bool (rect_t b0, rect_t b1)>;

    void generate(
        random_t& random
      , yama::map& map
      , room_generator& generator
      , region_connector& connector
    ) {
        split_(random, map.width(), map.height());

        int id = 0;
        for (auto const& region : regions_) {
            generator(region_id {id++}, region);
        }

        connect_(random, nodes_[0], connector);
    }
private:
    struct node_ {
        using index_t = int;

        enum : index_t { invalid = index_t{-1} };

        explicit node_(rect_t const Bounds) : bounds {Bounds} {}
        node_() : node_ {rect_t {}} {}

        bool is_leaf() const {
            return (first == invalid) && (second == invalid);
        }

        index_t first  {invalid};
        index_t second {invalid};
        rect_t  bounds;
    };

    bool do_split_(random_t& random, rect_t const bounds) const {
        return true;
    }

    void split_(random_t& random, positive<int> width, positive<int> height) {
        nodes_.clear();
        nodes_.emplace_back(rect_t{0, 0, width, height});

        size_t leaf_count = 1;
        for (auto i = decltype(nodes_)::size_type{0}; i < nodes_.size(); ++i) {
            BK_ASSERT(nodes_[i].is_leaf());
            auto const bounds = nodes_[i].bounds;

            if (!do_split_(random, bounds)) {
                continue;
            }

            auto const result = split_(random, bounds);

            nodes_.emplace_back(std::get<1>(result));
            nodes_.emplace_back(std::get<2>(result));

            auto const n    = nodes_.size();
            auto&      node = nodes_[i];

            node.first  = n - 2;
            node.second = n - 1;

            leaf_count++;
        }

        regions_.clear();
        regions_.reserve(leaf_count);

        for (auto& node : nodes_) {
            if (node.is_leaf()) {
                regions_.push_back(node.bounds);
            }
        }
    }

    static generate::split_result split_(random_t& random, rect_t const bounds) {
        auto const min_w     = 4;
        auto const min_h     = 4;
        auto const threshold = 1.0f;

        auto const type = generate::get_split_type(random, bounds, min_w, min_h, threshold);
        return generate::split_rect(random, bounds, type, min_w, min_h);
    }

    rect_t connect_(random_t& random, node_ const& node, region_connector& connector) const {
        if (!node.is_leaf()) {
            auto const a = connect_(random, nodes_[node.first],  connector);
            auto const b = connect_(random, nodes_[node.second], connector);

            connector(a, b);
        }

        return node.bounds;
    }

    std::vector<node_>  nodes_;
    std::vector<rect_t> regions_;
};


} //namespace yama


