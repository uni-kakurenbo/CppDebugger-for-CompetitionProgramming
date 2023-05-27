#pragma once


#include "internal/dev_env.hpp"

#include "data_structure/internal/declarations.hpp"

#include "data_structure/range_action/base.hpp"
#include "data_structure/range_action/flags.hpp"

#include "algebraic/addition.hpp"


namespace lib {

namespace actions {


template<class T> struct range_sum : base<> {
    static constexpr flags tags{ flags::range_folding };

    using operand = algebraic::addition<T>;
};


} // namespace actions


template<class T> struct fenwick_tree<actions::range_sum<T>> : internal::fenwick_tree_impl::core<actions::range_sum<T>,void> {
  private:
    using base = internal::fenwick_tree_impl::core<actions::range_sum<T>,void>;

  public:
    using base::base;
    using size_type = typename base::size_type;

    struct point_reference : base::point_reference {
        using base::point_reference::point_reference;

        inline point_reference& add(const T& val) noexcept(DEV_ENV) { this->_super->apply(this->_pos, val); return *this; }
        inline point_reference& operator+=(const T& val) noexcept(DEV_ENV) { this->_super->apply(this->_pos, val); return *this; }
        inline point_reference& operator-=(const T& val) noexcept(DEV_ENV) { this->_super->apply(this->_pos, -val); return *this; }
        inline point_reference& operator++(int) noexcept(DEV_ENV) { return *this += 1; }
        inline point_reference& operator--(int) noexcept(DEV_ENV) { return *this -= 1; }
        inline point_reference& operator++() noexcept(DEV_ENV) { const auto res = *this; *this += 1; return res; }
        inline point_reference& operator--() noexcept(DEV_ENV) { const auto res = *this; *this -= 1; return res; }
    };

    struct range_reference : base::range_reference {
        using base::range_reference::range_reference;

        inline auto sum() const { return this->_super->fold(this->_begin, this->_end); }
    };

    inline point_reference operator[](const size_type p) noexcept(DEV_ENV) { return point_reference(this, p); }
    inline range_reference operator()(const size_type l, const size_type r) noexcept(DEV_ENV) { return range_reference(this, l, r); }

    inline auto& add(const size_type first, const size_type last, const T& val) noexcept(DEV_ENV) { this->apply(first, last, val); return *this; }
    inline auto& add(const size_type pos, const T& val) noexcept(DEV_ENV) { this->apply(pos, val); return *this; }
    inline auto& add(const T& val) noexcept(DEV_ENV) { this->apply(val); return *this; }
    inline auto& operator+=(const T& val) noexcept(DEV_ENV) { this->apply(this->_pos, val); return *this; }
    inline auto& operator-=(const T& val) noexcept(DEV_ENV) { this->apply(this->_pos, -val); return *this; }
    inline auto& operator++(int) noexcept(DEV_ENV) { return *this += 1; }
    inline auto& operator--(int) noexcept(DEV_ENV) { return *this -= 1; }
    inline auto& operator++() noexcept(DEV_ENV) { const auto res = *this; *this += 1; return res; }
    inline auto& operator--() noexcept(DEV_ENV) { const auto res = *this; *this -= 1; return res; }

    inline auto sum(const size_t first, const size_t last) noexcept(DEV_ENV) { return this->fold(first, last); }
    inline auto sum() noexcept(DEV_ENV) { return this->fold(); }
};


} // namespace lib
