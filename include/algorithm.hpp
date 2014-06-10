#pragma once

namespace yama {

////////////////////////////////////////////////////////////////////////////////
//! for each value v in [begin, end) that satifies predicate(v), call functor(v).
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename P, typename F>
void for_each_where(T begin, T end, P predicate, F functor) {
    for (auto it = begin; it != end; ++it) {
        if (predicate(*it)) {
            functor(*it);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//! for each value v in cont that satifies predicate(v), call functor(v).
////////////////////////////////////////////////////////////////////////////////
template <typename Cont, typename P, typename F>
void for_each_where(Cont&& cont, P predicate, F functor) {
    for_each_where(std::begin(cont), std::end(cont), predicate, functor);
}

} //namespace yama
