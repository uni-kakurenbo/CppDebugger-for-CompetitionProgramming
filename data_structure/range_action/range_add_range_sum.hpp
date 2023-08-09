#pragma once


#include "internal/dev_env.hpp"

#include "data_structure/internal/declarations.hpp"

#include "data_structure/range_action/base.hpp"
#include "data_structure/range_action/flags.hpp"

#include "algebraic/addition.hpp"


namespace lib {

namespace actions {


template<class T> struct range_add_range_sum : base<> {
    static constexpr flags tags{ flags::range_folding, flags::range_operation };

    using operand = algebraic::addition<T>;
    using operation = algebraic::addition<T>;

    static operand map(const operand& x, const operation& y) noexcept(NO_EXCEPT) { return x.val() + y.val(); }
    static operation fold(const operation& x, const lib::internal::size_t length) noexcept(NO_EXCEPT) { return x.val() * length; }
};


} // namespace actions

} // namespace lib
