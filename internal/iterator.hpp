#pragma once


#include <utility>
#include <type_traits>
#include <iterator>
#include <variant>

#include "internal/dev_env.hpp"

#include "internal/types.hpp"
#include "internal/type_traits.hpp"

#include "numeric/arithmetic.hpp"


#if CPP20

#include <ranges>

#endif


namespace lib {

namespace internal {


template<class T>
struct iterator_interface {
    using iterator_category = std::output_iterator_tag;

    using difference_type = size_t;
    using value_type = T;

    using pointer = T*;
    using reference = T&;

    // virtual T operator*() const noexcept(NO_EXCEPT) { return 0; };
};

template<class T>
struct forward_iterator : iterator_interface<T> {
    using iterator_category = std::forward_iterator_tag;

    // virtual bidirectional_iterator_interface& operator++() = 0;
};

template<class T>
struct bidirectional_iterator_interface : forward_iterator<T> {
    using iterator_category = std::bidirectional_iterator_tag;

    // virtual bidirectional_iterator_interface& operator--() = 0;
};

template<class T>
struct random_access_iterator_base : bidirectional_iterator_interface<T> {
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = typename bidirectional_iterator_interface<T>::difference_type;

  public:
    // virtual random_access_iterator_base& operator+=(const difference_type count) = 0;
    // virtual random_access_iterator_base& operator-=(const difference_type count) = 0;

    friend inline random_access_iterator_base operator+(random_access_iterator_base itr, const difference_type count) noexcept(NO_EXCEPT) { return itr += count, itr; }
    friend inline random_access_iterator_base operator-(random_access_iterator_base itr, const difference_type count) noexcept(NO_EXCEPT) { return itr -= count, itr; }

};

template<class T, class container>
struct container_iterator_interface : random_access_iterator_base<T> {
    using difference_type = typename bidirectional_iterator_interface<T>::difference_type;

  protected:
    const container* _ref;
    difference_type _pos;

  public:
    container_iterator_interface() noexcept = default;
    container_iterator_interface(const container *const ref, const difference_type& pos) noexcept(NO_EXCEPT) : _ref(ref), _pos(pos) {}

    inline const container* ref() const noexcept(NO_EXCEPT) { return this->_ref; }

    inline difference_type pos() const noexcept(NO_EXCEPT) { return this->_pos; }
    inline difference_type& pos() { return this->_pos; }

    inline container_iterator_interface& operator++() noexcept(NO_EXCEPT) { return ++this->pos(), *this; }
    inline container_iterator_interface& operator--() noexcept(NO_EXCEPT) { return --this->pos(), *this; }

    inline container_iterator_interface operator++(int) noexcept(NO_EXCEPT) { const auto res = *this; return ++this->pos(), res; }
    inline container_iterator_interface operator--(int) noexcept(NO_EXCEPT) { const auto res = *this; return --this->pos(), res; }

    inline container_iterator_interface& operator+=(const difference_type count) noexcept(NO_EXCEPT) { return this->pos() += count, *this; }
    inline container_iterator_interface& operator-=(const difference_type count) noexcept(NO_EXCEPT) { return this->pos() -= count, *this; }

    inline auto operator*() const noexcept(NO_EXCEPT) { return this->ref()->get(this->pos()); }

    inline difference_type operator-(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return this->pos() - other.pos(); }

    inline bool operator<(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return *this - other < 0; }
    inline bool operator>(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return *this - other > 0; }

    inline bool operator<=(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return not (*this > other); }
    inline bool operator>=(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return not (*this < other); }

    inline bool operator!=(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return this->ref() != other.ref() or *this < other or *this > other; }
    inline bool operator==(const container_iterator_interface& other) const noexcept(NO_EXCEPT) { return not (*this != other); }
};

template<class V, class I>
inline auto to_non_const_iterator(V v, const I itr) noexcept(NO_EXCEPT) { return std::next(std::begin(v), std::distance(std::cbegin(v), itr)); }


namespace iterator_impl {


template<class... Tags>
using is_all_random_access_iterator = are_base_of<std::random_access_iterator_tag,Tags...>;

template<class... Tags>
using is_all_bidirectional_iterator = are_base_of<std::bidirectional_iterator_tag,Tags...>;

template<class... Tags>
using is_all_forward_iterator = are_base_of<std::forward_iterator_tag,Tags...>;

template<class... Tags>
using is_all_input_iterator = are_base_of<std::input_iterator_tag,Tags...>;


template<class... Tags>
constexpr auto _most_primitive_iterator_tag() {
    if constexpr(is_all_random_access_iterator<Tags...>::value) {
        return std::random_access_iterator_tag{};
    }
    else if constexpr(is_all_bidirectional_iterator<Tags...>::value) {
        return std::bidirectional_iterator_tag{};
    }
    else if constexpr(is_all_forward_iterator<Tags...>::value) {
        return std::forward_iterator_tag{};
    }
    else {
        return std::input_iterator_tag{};
    }
}


} // namespace iterator_impl


template<class... Tags>
using most_primitive_iterator_tag = decltype(iterator_impl::_most_primitive_iterator_tag<Tags...>());


template<class T, class = void>
struct is_iterator {
   static constexpr bool value = false;
};

template<class T>
struct is_iterator<T, typename std::enable_if<!std::is_same<typename std::iterator_traits<T>::value_type, void>::value>::type> {
   static constexpr bool value = true;
};

template<class T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<class T>
using is_iterator_t = std::enable_if_t<is_iterator_v<T>>;

template<class T>
using iota_diff_t = std::make_signed_t<T>;


} // namespace internal

} // namespace lib
