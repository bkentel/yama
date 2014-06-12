////////////////////////////////////////////////////////////////////////////////
//! @file
//! Checked, policy based, values.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <type_traits>
#include "assert.hpp" //TODO remove

namespace yama {

enum class failure_policy {
    ignore          //!< ignore the violation
  , clamp           //!< clamp the value to some valid value
  , abort           //!< abort execution
  , throw_exception //!< throw an exception
};

static constexpr auto default_failure_policy = failure_policy::abort;

inline void failure_abort_impl() {
    BK_DEBUG_BREAK();
    std::terminate();
}

void failure_throw_exception_impl();

template <
    typename       Base   //!< CRTP base class actually implementing the check.
  , failure_policy Policy //!< The policy type.
>
struct apply_failure_policy;

template <typename Base>
struct apply_failure_policy<Base, failure_policy::ignore> {
    template <typename T>
    static void apply(T const&) {
    }
};

template <typename Base>
struct apply_failure_policy<Base, failure_policy::abort> {
    template <typename T>
    static void apply(T const& value) {
        if (!Base::check(value)) {
            failure_abort_impl();
        }
    }
};

template <
    typename Base  //!< CRTP base class actually implementing the check.
>
struct check_base {
    template <failure_policy Policy, typename T>
    static void apply(T&& value) {
        apply_failure_policy<Base, Policy>::apply(std::forward<T>(value));
    }
};

////////////////////////////////////////////////////////////////////////////////
//! @tpara T The value type.
//! @tpara Check The check to perform on the value.
//! @tpara Policy The failure policy for the check.
////////////////////////////////////////////////////////////////////////////////
template <
    typename       T
  , typename       Check
  , failure_policy Policy = default_failure_policy
>
class checked_value {
public:
    using value_type = T;

    checked_value(T const value = T{})
      : value_ {value}
    {
        Check::template apply<Policy>(value_);
    }

    template <typename R>
    checked_value(R const value)
      : checked_value {static_cast<T>(value)}
    {
    }

    template <typename R>
    checked_value& operator=(R const rhs) {
        value_ = static_cast<T>(rhs);
        Check::template apply<Policy>(value_);
        return *this;
    }

    bool operator<(T const rhs)  const { return value_ <  rhs; }
    bool operator<=(T const rhs) const { return value_ <= rhs; }
    bool operator>(T const rhs)  const { return value_ >  rhs; }
    bool operator>=(T const rhs) const { return value_ >= rhs; }
    bool operator==(T const rhs) const { return value_ == rhs; }
    bool operator!=(T const rhs) const { return value_ != rhs; }

    operator T const&() const { return value_; }
    operator T&()             { return value_; }
private:
    T value_;
};


////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! Get an integral type compatible with T.
////////////////////////////////////////////////////////////////////////////////
template <typename T>
using get_scalar_type_t = std::conditional_t<
    std::is_signed<T>::value
  , signed
  , unsigned
>;


////////////////////////////////////////////////////////////////////////////////
//! Check that a value >= Minimum.
////////////////////////////////////////////////////////////////////////////////
template <typename T, get_scalar_type_t<T> Minimum>
struct check_minimum : check_base<check_minimum<T, Minimum>> {
    static_assert(std::is_arithmetic<T>::value, "");

    template <typename U>
    static bool check(U const& value) {
        return value >= U{Minimum};
    }
};

////////////////////////////////////////////////////////////////////////////////
//! Check that value satisfies: Minimum <= value <= Maximum.
//! @pre Minimum <= Maximum
////////////////////////////////////////////////////////////////////////////////
template <typename T, get_scalar_type_t<T> Minimum, get_scalar_type_t<T> Maximum>
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
using positive = checked_value<T, check_minimum<T, 0>>;

////////////////////////////////////////////////////////////////////////////////
//! A positive value >= 0.
//! @tparam T The value type.
////////////////////////////////////////////////////////////////////////////////
template <typename T = float>
using aspect_ratio = checked_value<T, check_minimum<T, 1>>;

} //namespace yama;
