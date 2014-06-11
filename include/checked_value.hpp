////////////////////////////////////////////////////////////////////////////////
//! @file
//! Checked, policy based, values.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <type_traits>
#include "assert.hpp"

namespace yama {

enum class failure_policy {
    ignore          //!< ignore the violation
  , clamp           //!< clamp the value to some valid value
  , abort           //!< abort execution
  , throw_exception //!< throw an exception
};

static constexpr auto default_failure_policy = failure_policy::abort;

template <typename T, typename Check, failure_policy Policy = default_failure_policy>
class checked_value {
public:
    checked_value(T const value = T{})
      : value_ {value}
    {
        Check::template apply<Policy>(value_);
    }

    operator T const&() const { return value_; }
    operator T&()             { return value_; }
private:
    T value_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Base, failure_policy Policy>
struct failure_policy_base;

template <typename Base>
struct failure_policy_base<Base, failure_policy::ignore> {
    template <typename T>
    static void apply(T&&) {}
};

template <typename Base>
struct failure_policy_base<Base, failure_policy::abort> {
    template <typename T>
    static void apply(T const& value) {
        BK_ASSERT(Base::check(value));
    }
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
using get_constant_type_t = std::conditional_t<
    std::is_signed<T>::value
  , signed
  , unsigned
>;

////////////////////////////////////////////////////////////////////////////////

template <typename Base>
struct check_base {
    template <failure_policy Policy, typename T>
    static void apply(T&& value) {
        failure_policy_base<Base, Policy>::apply(value);
    }
};

struct check_allow_any {
    template <typename T>
    static bool check(T const&) {
        return true;
    }
};

template <typename T, get_constant_type_t<T> Minimum>
struct check_minimum : check_base<check_minimum<T, Minimum>> {
    static_assert(std::is_arithmetic<T>::value, "");

    template <typename U>
    static bool check(U const& value) {
        return value >= U{Minimum};
    }
};

template <typename T, get_constant_type_t<T> Minimum, get_constant_type_t<T> Maximum>
struct check_closed_range : check_base<check_closed_range<T, Minimum, Maximum>> {
    static_assert(std::is_arithmetic<T>::value, "");
    static_assert(Minimum <= Maximum, "");

    template <typename U>
    static bool check(U const& value) {
        return value >= U{Minimum} && value <= U{Maximum};
    }
};

////////////////////////////////////////////////////////////////////////////////
//! A strict [0, 100] percentage value.
//! @tparam T The value type.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
using strict_percentage = checked_value<T, check_closed_range<T, 0, 100>>;

////////////////////////////////////////////////////////////////////////////////
//! A positive value >= 0.
//! @tparam T The value type.
////////////////////////////////////////////////////////////////////////////////
template <typename T = int>
using positive2 = checked_value<T, check_minimum<T, 0>>;

////////////////////////////////////////////////////////////////////////////////
//! A positive value >= 0.
//! @tparam T The value type.
////////////////////////////////////////////////////////////////////////////////
template <typename T = float>
using aspect_ratio2 = checked_value<T, check_minimum<T, 1>>;

} //namespace yama;
