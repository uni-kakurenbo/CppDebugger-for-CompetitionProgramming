#pragma once

#include <cassert>
#include <vector>
#include <iterator>
#include <utility>
#include <type_traits>

#include "internal/dev_env.hpp"
#include "internal/types.hpp"
#include "internal/iterator.hpp"
#include "internal/point_reference.hpp"
#include "internal/range_reference.hpp"

#include "snippet/iterations.hpp"
#include "numeric/bit.hpp"

#include "data_structure/range_action/flags.hpp"
#include "data_structure/internal/is_action.hpp"
#include "algebraic/internal/traits.hpp"


namespace lib {

namespace internal {

namespace fenwick_tree_impl {


// Thanks to: atcoder::fenwick_tree
template<class S>
struct base {
    using size_type = internal::size_t;

  private:
    size_type _n = 0, _bit_ceil = 0;
    S *const _data = nullptr;

  protected:
    base() noexcept(NO_EXCEPT) {}

    inline void initialize() noexcept(NO_EXCEPT) {
        FOR(i, 1, this->_n) {
            size_type j = i + (i & -i);
            if(j <= this->_n) this->_data[j-1] = this->_data[j-1] + this->_data[i-1];
        }
    }

    explicit base(const size_type n) noexcept(NO_EXCEPT)
      : _n(n), _bit_ceil(lib::bit_ceil<std::make_unsigned_t<size_type>>(n)), _data(new S[n]())
    {}

    ~base() { delete[] this->_data; }

    inline size_type size() const noexcept(NO_EXCEPT) { return this->_n; }

    inline S* data() noexcept(NO_EXCEPT) { return this->_data; }
    inline const S* data() const noexcept(NO_EXCEPT) { return this->_data; }

    inline void apply(size_type p, const S& x) noexcept(NO_EXCEPT) {
        for(p++; p<=this->_n; p += p & -p) this->_data[p-1] = this->_data[p-1] + x;
    }

    inline void set(const size_type p, const S& x) noexcept(NO_EXCEPT) {
        assert(this->get(p) == this->fold(p, p+1));
        this->apply(p, x + -this->get(p));
    }

    inline S fold(size_type r) const noexcept(NO_EXCEPT) {
        S s = S{};
        for(; r>0; r -= r & -r) s = s + this->_data[r-1];
        return s;
    }
    inline S fold(size_type l, size_type r) const noexcept(NO_EXCEPT) {
        S s = S{};
        for(; l < r; r -= r & -r) s = s + this->_data[r-1];
        for(; r < l; l -= l & -l) s = s + -this->_data[l-1];
        return s;
    }

    inline S get(size_type p) const noexcept(NO_EXCEPT) { return this->fold(p, p+1); }

  public:
    template<class F> inline size_type max_right(size_type l, const F& f) const noexcept(NO_EXCEPT) {
        assert(0 <= l && l <= this->_n);
        assert(f(S{}));
        if(l == this->_n) return this->_n;
        S fold_l_inv = -this->fold(l);
        size_type p = 0, q = this->_bit_ceil;
        for(size_type k=q; k>0; k >>= 1) {
            if(p+k <= this->_n and f(this->_data[p+k-1] + fold_l_inv)) {
                fold_l_inv = fold_l_inv + this->_data[(p+=k)-1];
            }
        }
        return p;
    }

    template<class F> inline size_type min_left(size_type r, const F& f) const noexcept(NO_EXCEPT) {
        assert(0 <= r && r <= this->_n);
        assert(f(S{}));
        if(r == 0) return 0;
        S fold_r = this->fold(r);
        size_type p = 0, q = lib::bit_ceil<std::make_unsigned_t<size_type>>(r);
        for(size_type k=q; k>0; k >>= 1) {
            if(p+k < r and !f(fold_r + -this->_data[p+k-1])) {
                fold_r = fold_r + -this->_data[(p+=k)-1];
            }
        }
        if(p == 0 and f(fold_r)) return 0;
        return p + 1;
    }
};


template<class, class = std::void_t<>> struct core {};

template<class monoid>
struct core<monoid,std::void_t<typename algebraic::internal::is_monoid_t<monoid>>> : base<monoid> {
    static_assert(algebraic::internal::is_commutative_v<monoid>, "commutative property is required");

  private:
    using base = typename fenwick_tree_impl::base<monoid>;

  public:
    using value_type = monoid;
    using size_type = typename base::size_type;

  protected:
    inline size_type _positivize_index(const size_type p) const noexcept(NO_EXCEPT) {
        return p < 0 ? this->size() + p : p;
    }

  public:
    core() noexcept(NO_EXCEPT) : base() {}
    explicit core(const size_type n, const value_type& v = {}) noexcept(NO_EXCEPT) : base(n) { this->fill(v); }
    template<class T> core(const std::initializer_list<T>& init_list) noexcept(NO_EXCEPT) : core(ALL(init_list)) {}

    template<class I, std::void_t<typename std::iterator_traits<I>::value_type>* = nullptr>
    explicit core(const I first, const I last) noexcept(NO_EXCEPT) : core(static_cast<size_type>(std::distance(first, last))) { this->assign(first, last); }


    template<class T>
    inline auto& assign(const std::initializer_list<T>& init_list) noexcept(NO_EXCEPT){ return this->assign(ALL(init_list)); }

    template<class I, std::void_t<typename std::iterator_traits<I>::value_type>* = nullptr>
    inline auto& assign(const I first, const I last) noexcept(NO_EXCEPT) {
        assert(std::distance(first, last) == this->size());
        std::copy(first, last, this->data());
        this->initialize();
        return *this;
    }

    inline auto& fill(const value_type& v = {}) noexcept(NO_EXCEPT) {
        std::fill(this->data(), this->data() + this->size(), v);
        this->initialize();
        return *this;
    }

    inline size_type size() const noexcept(NO_EXCEPT) { return this->base::size(); }
    inline bool empty() const noexcept(NO_EXCEPT) { return this->base::size() == 0; }

    struct point_reference : internal::point_reference<core> {
        point_reference(core *const super, const size_type p) noexcept(NO_EXCEPT)
          : internal::point_reference<core>(super, super->_positivize_index(p))
        {
            assert(0 <= this->_pos && this->_pos < this->_super->size());
        }

        operator value_type() const noexcept(NO_EXCEPT) { return this->_super->get(this->_pos); }
        value_type val() const noexcept(NO_EXCEPT) { return this->_super->get(this->_pos); }

        inline point_reference& set(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->set(this->_pos, v);
            return *this;
        }
        inline point_reference& operator=(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->set(this->_pos, v);
            return *this;
        }

        inline point_reference& apply(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->apply(this->_pos, v);
            return *this;
        }
        inline point_reference& operator<<=(const value_type& v) noexcept(NO_EXCEPT) {
            this->_super->apply(this->_pos, v);
            return *this;
        }
    };

    struct range_reference : internal::range_reference<core> {
        range_reference(core *const super, const size_type l, const size_type r) noexcept(NO_EXCEPT)
          : internal::range_reference<core>(super, super->_positivize_index(l), super->_positivize_index(r))
        {
            assert(0 <= this->_begin && this->_begin <= this->_end && this->_end <= this->_super->size());
        }

        inline value_type fold() noexcept(NO_EXCEPT) {
            if(this->_begin == 0 and this->_end == this->_super->size()) return this->_super->fold();
            if(this->_begin == 0) return this->_super->fold(this->_end);
            return this->_super->fold(this->_begin, this->_end);
        }
        inline value_type operator*() noexcept(NO_EXCEPT) {
            if(this->_begin == 0 and this->_end == this->_super->size()) return this->_super->fold();
            if(this->_begin == 0) return this->_super->fold(this->_end);
            return this->_super->fold(this->_begin, this->_end);
        }
    };


    inline auto& apply(const size_type p, const value_type& x) noexcept(NO_EXCEPT) {
        assert(0 <= p && p < this->size());
        this->base::apply(p, x);
         return *this;
    }

    inline auto& set(const size_type p, const value_type& x) noexcept(NO_EXCEPT) {
        static_assert(algebraic::internal::is_invertible_v<value_type>, "point setting requires inverse element");
        assert(0 <= p && p < this->size());
        this->base::set(p, x);
         return *this;
    }

    inline value_type get(const size_type p) const noexcept(NO_EXCEPT) {
        static_assert(algebraic::internal::is_invertible_v<value_type>, "point getting requires inverse element");
        assert(0 <= p && p < this->size());
        return this->base::get(p);
    }

    inline point_reference operator[](const size_type p) noexcept(NO_EXCEPT) { return point_reference(this, p); }

    inline const range_reference operator()(const size_type l, const size_type r) const noexcept(NO_EXCEPT) { return range_reference(this, l, r); }
    inline range_reference operator()(const size_type l, const size_type r) noexcept(NO_EXCEPT) { return range_reference(this, l, r); }

    inline value_type fold(const size_type l, const size_type r) const noexcept(NO_EXCEPT) {
        static_assert(algebraic::internal::is_invertible_v<value_type>, "range folding requires an inverse element");
        assert(0 <= l && l <= r && r <= this->size());
        return this->base::fold(l, r);
    }
    inline value_type fold(const size_type r) const noexcept(NO_EXCEPT) {
        assert(0 <= r && r <= this->size());
        return this->base::fold(r);
    }
    inline value_type fold() const noexcept(NO_EXCEPT) {
        return this->base::fold(this->size());
    }


  protected:
    using iterator_interface = internal::container_iterator_interface<value_type,core>;

  public:
    struct iterator : virtual iterator_interface {
        iterator(const core *const ref, const size_type p) noexcept(NO_EXCEPT) : iterator_interface(ref, p) {}

        inline value_type operator*() const noexcept(NO_EXCEPT) { return this->ref()->get(this->pos()); }
        inline value_type operator[](const typename iterator_interface::difference_type count) const noexcept(NO_EXCEPT) { return *(*this + count); }
    };

    inline iterator begin() const noexcept(NO_EXCEPT) { return iterator(this, 0); }
    inline iterator end() const noexcept(NO_EXCEPT) { return iterator(this, this->size()); }
};

template<class Action>
struct core<Action, std::void_t<typename internal::is_action_t<Action>>> : core<typename Action::operand> {
    using action = Action;
    using core<typename action::operand>::core;

    static_assert(action::tags.none() or action::tags.has(actions::flags::range_folding));
};


} // namespace fenwick_tree_impl

} // namespace internal


template<class action> struct fenwick_tree : internal::fenwick_tree_impl::core<action> {
    using internal::fenwick_tree_impl::core<action>::core;
};


} // namespace lib
