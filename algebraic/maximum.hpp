#pragma once


#include <limits>
#include <functional>

#include "internal/dev_env.hpp"
#include "algebraic/base.hpp"


namespace lib {

namespace algebraic {


template<class T> struct maximum : base<T>, monoid, commutative {
    using base<T>::base;
    maximum() noexcept(NO_EXCEPT) : base<T>(std::numeric_limits<T>::lowest()) {};
    friend inline maximum operator+(const maximum& lhs, const maximum& rhs) noexcept(NO_EXCEPT) { return std::max(lhs.val(), rhs.val()); }
};


} // namespace algebraic

} // namespace lib
