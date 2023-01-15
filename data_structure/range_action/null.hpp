#pragma once


#include "data_structure/internal/declarations.hpp"

#include "data_structure/range_action/base.hpp"
#include "data_structure/range_action/flags.hpp"

#include "data_structure/monoid/addition.hpp"
#include "data_structure/monoid/null.hpp"

#include "internal/exception.hpp"


namespace lib {

namespace actions {


template<class T> struct null : base<monoids::null<T>> {
    static constexpr flags tags{ flags::implicit_treap };

    using operand_monoid = monoids::addition<T>;
    using operator_monoid = monoids::null<T>;

    static operand_monoid map(const operand_monoid& x, const operator_monoid&) { return x; }
};


} // namespace actions


template<class T> struct implicit_treap<actions::null<T>> : internal::implicit_treap_lib::core<actions::null<T>> {
    using internal::implicit_treap_lib::core<actions::null<T>>::core;

    template<class... Args> void apply(const Args&... args) = delete;
    template<class... Args> void prod(const Args&... args) = delete;
};


} // namespace lib
