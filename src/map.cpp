#include "pch.hpp"
#include "map.hpp"

#include "grid.hpp"

using yama::map;

class map::impl_t {
public:
    impl_t(int const Width, int const Height)
      : category_ {Width, Height}
    {
    }

    void clear() {
        category_.clear();
    }

    tile_category get_category(int x, int y) const {
        return category_(x, y);
    }

    void set_category(int x, int y, tile_category const value) {
        category_(x, y) = value;
    }

    bool is_valid_position(int x, int y) const {
        return category_.is_valid_index(x, y);
    }

    int width() const {
        return category_.width();
    }

    int height() const {
        return category_.height();
    }
private:
    grid<tile_category> category_;
};

/////////////////////

map::map(map&& other)
  : impl_ {std::move(other.impl_)}
{
}

map& map::operator=(map&& rhs) {
    std::swap(impl_, rhs.impl_);
    return *this;
}

map::map(yama::map_size const Width, yama::map_size const Height)
  : impl_ {std::make_unique<impl_t>(Width, Height)}
{
}

map::~map() {
}

void map::clear() {
    impl_->clear();
}

yama::tile_category map::get_category_(int const x, int const y) const {
    return impl_->get_category(x, y);
}

void map::set_category_(int x, int y, yama::tile_category value) {
    impl_->set_category(x, y, value);
}

bool map::is_valid_position(int x, int y) const {
    return impl_->is_valid_position(x, y);
}

int map::width() const {
    return impl_->width();
}

int map::height() const {
    return impl_->height();
}
