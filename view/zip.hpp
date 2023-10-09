#pragma once


#include "internal/dev_env.hpp"
#include "internal/ranges.hpp"
#include "internal/type_traits.hpp"

#include "utility/functional.hpp"

// befor C++20
#include "internal/dev_env.hpp"
#include "internal/types.hpp"
#include "internal/iterator.hpp"
#include "internal/type_traits.hpp"

#include "view/range.hpp"
#include "view/internal/base.hpp"
//


#if CPP20


#include <ranges>
#include <concepts>
#include <tuple>
#include <iterator>


namespace lib {


template<std::ranges::input_range... Views> requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
struct zip_view : std::ranges::view_interface<zip_view<Views...>> {
  private:
    std::tuple<Views...> _views;

  public:
    template<bool> struct iterator;
    template<bool> struct sentinel;

    zip_view() = default;

    constexpr explicit zip_view(Views... __views) noexcept(NO_EXCEPT) : _views(std::move(__views)...) {}

    constexpr auto begin() noexcept(NO_EXCEPT) requires(!(internal::simple_view<Views> && ...))
    {
        return iterator<false>(tuple_transform(std::ranges::begin, this->_views));
    }

    constexpr auto begin() const noexcept(NO_EXCEPT) requires(std::ranges::range<const Views> && ...)
    {
        return iterator<true>(tuple_transform(std::ranges::begin, this->_views));
    }

    constexpr auto end() noexcept(NO_EXCEPT) requires(!(internal::simple_view<Views> && ...))
    {
        if constexpr(!internal::zip_is_common<Views...>)
            return sentinel<false>(tuple_transform(std::ranges::end, this->_views));
        else if constexpr((std::ranges::random_access_range<Views> && ...))
            return begin() + std::iter_difference_t<iterator<false>>(this->size());
        else
            return iterator<false>(tuple_transform(std::ranges::end, this->_views));
    }

    constexpr auto end() const noexcept(NO_EXCEPT) requires(std::ranges::range<const Views> && ...)
    {
        if constexpr(!internal::zip_is_common<const Views...>)
            return sentinel<true>(tuple_transform(std::ranges::end, this->_views));
        else if constexpr((std::ranges::random_access_range<const Views> && ...))
            return this->begin() + std::iter_difference_t<iterator<true>>(this->size());
        else
            return iterator<true>(tuple_transform(std::ranges::end, _views));
    }

    constexpr auto size() noexcept(NO_EXCEPT) requires(std::ranges::sized_range<Views> && ...)
    {
        return std::apply(
            [](auto... sizes)
            {
                using size_type = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
                return lib::min(size_type(sizes)...);
            },
           tuple_transform(std::ranges::size, _views)
        );
    }

    constexpr auto size() const noexcept(NO_EXCEPT) requires(std::ranges::sized_range<const Views> && ...)
    {
        return std::apply(
            [](auto... sizes)
            {
                using size_type = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
                return lib::min(size_type(sizes)...);
            },
           tuple_transform(std::ranges::size, _views)
        );
    }
};

template<class... Ranges> zip_view(Ranges &&...) -> zip_view<std::views::all_t<Ranges>...>;

namespace internal {


template<class iterator>
constexpr const typename iterator::iterator_collection& get_current(const iterator& itr) noexcept(NO_EXCEPT);


} // namespace internal


template<std::ranges::input_range... Views> requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
template<bool Const>
struct zip_view<Views...>::iterator
    : internal::zip_view_iterator_category<Const, Views...> {
    using iterator_collection = internal::tuple_or_pair_t<
            std::ranges::iterator_t<internal::maybe_const_t<Const, Views>>...
        >;

  private:
    friend struct zip_view;
    template<bool> friend struct zip_view::sentinel;

    iterator_collection _current;

    constexpr explicit iterator(decltype(_current) __current) : _current(std::move(__current)) {}

    // template<std::copy_constructible F, std::ranges::input_range... Vs>
    //     requires
    //         (std::ranges::view<Vs> && ...) && (sizeof...(Vs) > 0) && std::is_object_v<F> &&
    //         std::regular_invocable<F&, std::ranges::range_reference_t<Vs>...> &&
    //         internal::can_reference<std::invoke_result_t<F&, std::ranges::range_reference_t<Vs>...>>
    // friend struct zip_transform_view;


  public:
    using iterator_concept = internal::most_primitive_iterator_concept<Const, Views...>;

    using value_type = internal::tuple_or_pair_t<std::ranges::range_value_t<internal::maybe_const_t<Const, Views>>...>;

    using difference_type = std::common_type_t<std::ranges::range_difference_t<internal::maybe_const_t<Const, Views>>...>;

    iterator() = default;

    constexpr iterator(iterator<!Const> itr) noexcept(NO_EXCEPT)
        requires Const &&
                (
                    std::convertible_to<
                        std::ranges::iterator_t<Views>,
                        std::ranges::iterator_t<internal::maybe_const_t<Const, Views>>
                    > && ...
                )
      : _current(std::move(itr._current))
    {}


    constexpr auto operator*() const noexcept(NO_EXCEPT) {
        const auto f = [](auto &itr) -> decltype(auto) { return *itr; };
        return tuple_transform(f, this->_current);
    }

    constexpr iterator& operator++() noexcept(NO_EXCEPT) {
        tuple_for_each([](auto &itr) { ++itr; }, this->_current);
        return *this;
    }

    constexpr void operator++(int) noexcept(NO_EXCEPT) { ++*this; }

    constexpr iterator operator++(int) noexcept(NO_EXCEPT)
        requires internal::all_forward<Const, Views...>
    {
        const auto res = *this; ++*this; return res;
    }

    constexpr iterator& operator--() noexcept(NO_EXCEPT)
        requires internal::all_bidirectional<Const, Views...>
    {
        tuple_for_each([](auto &itr) { --itr; }, this->_current);
        return *this;
    }

    constexpr iterator operator--(int) noexcept(NO_EXCEPT)
        requires internal::all_bidirectional<Const, Views...>
    {
        const auto res = *this; --*this; return res;
    }

    constexpr iterator& operator+=(const difference_type diff) noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        const auto f = [&]<class Itr>(Itr& itr) constexpr noexcept(NO_EXCEPT) {
            itr += std::iter_difference_t<Itr>(diff);
        };
        tuple_for_each(f, this->_current);
        return *this;
    }

    constexpr iterator& operator-=(const difference_type diff) noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        const auto f = [&]<class Itr>(Itr& itr) constexpr noexcept(NO_EXCEPT) {
            itr -= std::iter_difference_t<Itr>(diff);
        };
        tuple_for_each(f, this->_current);
        return *this;
    }

    constexpr auto operator[](const difference_type diff) const noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        const auto f = [&]<class Itr>(Itr& itr) constexpr noexcept(NO_EXCEPT) -> decltype(auto) {
            return itr[iter_difference_t<Itr>(diff)];
        };
        returntuple_transform(f, _current);
    }

    friend constexpr bool operator==(const iterator& lhs, const iterator& rhs) noexcept(NO_EXCEPT)
        requires (
            std::equality_comparable<
                std::ranges::iterator_t<internal::maybe_const_t<Const, Views>>
            > && ...
        )
    {
        if constexpr(internal::all_bidirectional<Const, Views...>)
            return lhs._current == rhs._current;
        else
            return [&]<std::size_t... Is>(std::index_sequence<Is...>) constexpr noexcept(NO_EXCEPT) {
                return (
                    (std::get<Is>(lhs._current) == std::get<Is>(rhs._current)) || ...
                );
            }(std::make_index_sequence<sizeof...(Views)>{});
    }

    friend constexpr auto operator<=>(const iterator& lhs, const iterator& rhs) noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        return lhs._current <=> rhs._current;
    }

    friend constexpr iterator operator+(const iterator& itr, const difference_type diff) noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        auto res = itr; res += diff; return res;
    }

    friend constexpr iterator operator+(const difference_type diff, const iterator& itr) noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        auto res = itr; res += diff; return res;
    }

    friend constexpr iterator operator-(const iterator& itr, const difference_type diff) noexcept(NO_EXCEPT)
        requires internal::all_random_access<Const, Views...>
    {
        auto res = itr; res -= diff; return res;
    }

    friend constexpr difference_type operator-(const iterator& lhs, const iterator& rhs) noexcept(NO_EXCEPT)
        requires (
            std::sized_sentinel_for<
                std::ranges::iterator_t<internal::maybe_const_t<Const, Views>>,
                std::ranges::iterator_t<internal::maybe_const_t<Const, Views>>
            > && ...
        )
    {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) constexpr noexcept(NO_EXCEPT) {
            return std::ranges::min(
                {
                    difference_type(std::get<Is>(lhs._current) - std::get<Is>(rhs._current)) ...
                },
                std::ranges::less{},
                [](const difference_type diff) constexpr noexcept(NO_EXCEPT) {
                    return to_unsigned(diff < 0 ? -diff : diff);
                }
            );
        }(std::make_index_sequence<sizeof...(Views)>{});
    }

    friend constexpr auto iter_move(const iterator& itr) noexcept(NO_EXCEPT) {
        return tuple_transform(std::ranges::iter_move, itr._current);
    }

    friend constexpr void iter_swap(const iterator& lhs, const iterator& res) noexcept(NO_EXCEPT)
        requires (
            std::indirectly_swappable<
                std::ranges::iterator_t<internal::maybe_const_t<Const, Views>>
            > && ...
        )
    {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) constexpr noexcept(NO_EXCEPT) {
            (
                std::ranges::iter_swap(std::get<Is>(lhs._current), std::get<Is>(res._current)), ...
            );
        }(std::make_index_sequence<sizeof...(Views)>{});
    }

    template<class Itr> friend constexpr const typename Itr::iterator_collection& internal::get_current(const Itr&) noexcept(NO_EXCEPT);
};


template<class iterator>
constexpr const typename iterator::iterator_collection& internal::get_current(const iterator& itr) noexcept(NO_EXCEPT) { return itr._current; };


template<std::ranges::input_range... Views> requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
template<bool Const>
struct zip_view<Views...>::sentinel {
    friend struct zip_view;
    template<bool> friend struct zip_view::iterator;

    using sentinel_collection = internal::tuple_or_pair_t<
            std::ranges::sentinel_t<internal::maybe_const_t<Const, Views>>...
        >;

    sentinel_collection _end;

    constexpr explicit sentinel(decltype(_end) __end) noexcept(NO_EXCEPT) : _end(__end) {}

  public:
    sentinel() = default;

    constexpr sentinel(sentinel<!Const> itr) noexcept(NO_EXCEPT)
        requires Const &&
                (
                    std::convertible_to<
                        std::ranges::sentinel_t<Views>,
                        std::ranges::sentinel_t<internal::maybe_const_t<Const, Views>>
                    > && ...
                )
      : _end(std::move(itr._end))
    {}

    template<bool Const_>
        requires (
            std::sentinel_for<
                std::ranges::sentinel_t<internal::maybe_const_t<Const, Views>>,
                std::ranges::iterator_t<internal::maybe_const_t<Const_, Views>>
            > && ...
        )
    friend constexpr bool operator==(const iterator<Const_>& lhs, const sentinel& rhs) noexcept(NO_EXCEPT)
    {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) constexpr noexcept(NO_EXCEPT) {
            return (
                (std::get<Is>(internal::get_current(lhs)) == std::get<Is>(rhs._end)) || ...);
        }(std::make_index_sequence<sizeof...(Views)>{});
    }

    template<bool Const_>
        requires (
            std::sized_sentinel_for<
                std::ranges::sentinel_t<internal::maybe_const_t<Const, Views>>,
                std::ranges::iterator_t<internal::maybe_const_t<Const_, Views>>
            > && ...
        )
    friend constexpr auto operator-(const iterator<Const_>& lhs, const sentinel& rhs) noexcept(NO_EXCEPT)
    {
        using return_type = std::common_type_t<std::ranges::range_difference_t<internal::maybe_const_t<Const_, Views>>...>;
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) constexpr noexcept(NO_EXCEPT) {
            return std::ranges::min(
                { return_type(std::get<Is>(internal::get_current(lhs)) - std::get<Is>(rhs._end))... },
                std::ranges::less{},
                [](const return_type diff) {
                    return to_unsigned(diff < 0 ? -diff : diff);
                }
            );
        }(std::make_index_sequence<sizeof...(Views)>{});
    }

    template<bool Const_>
        requires (
            std::sized_sentinel_for<
                std::ranges::sentinel_t<internal::maybe_const_t<Const, Views>>,
                std::ranges::iterator_t<internal::maybe_const_t<Const_, Views>>
            > && ...
        )
    friend constexpr auto operator-(const sentinel &lhs, const iterator<Const_>& rhs) noexcept(NO_EXCEPT)
    {
        return -(rhs - lhs);
    }
};


namespace views {


namespace internal {


template<class... Ts>
concept can_zip_view = requires { zip_view<std::views::all_t<Ts>...>(std::declval<Ts>()...); };


} // namespace internal


struct Zip {
    template<class... Ts> requires(sizeof...(Ts) == 0 || internal::can_zip_view<Ts...>)
    constexpr auto operator() [[nodiscard]] (Ts&&... vs) const {
        if constexpr(sizeof...(Ts) == 0) return std::views::empty<std::tuple<>>;
        else return zip_view<std::views::all_t<Ts>...>(std::forward<Ts>(vs)...);
    }
};


inline constexpr Zip zip;


} // namespace views


} // namespace lib.


namespace std::ranges {

template<class... Views>
inline constexpr bool enable_borrowed_range<lib::zip_view<Views...>> = (enable_borrowed_range<Views> && ...);

}

#else


#include <iterator>
#include <utility>
#include <tuple>
#include <type_traits>
#include <functional>

namespace lib {


template<class...> struct zip_view;

namespace internal {

namespace view_impl {


template<class,class...> struct zip_iterator;

template<class View, class... Iterators>
struct zip_iterator : iterator_interface<std::tuple<typename std::iterator_traits<Iterators>::value_type...>>, iterator_base {
    using difference_type = size_t;
    using iterator_category = most_primitive_iterator_tag<class std::iterator_traits<Iterators>::iterator_category...>;

    friend View;

//   protected:
    using indices = std::make_index_sequence<sizeof...(Iterators)>;
    using iterator_collection = std::tuple<Iterators...>;

    iterator_collection _iterators;

    auto as_const() const noexcept(NO_EXCEPT) {
        return typename View::const_iterator(this->_iterators);
    }

  public:
    zip_iterator() = default;
    explicit zip_iterator(const Iterators... iterators) noexcept(NO_EXCEPT) : _iterators({ iterators... }) {}
    template<class... Is> zip_iterator(const std::tuple<Is...> coll) noexcept(NO_EXCEPT) : _iterators(coll) {}

    inline auto& operator++() noexcept(NO_EXCEPT) {
        std::apply([this](auto&... args) { (++args, ...); }, this->_iterators);
        return *this;
    }
    inline auto operator++(int) noexcept(NO_EXCEPT) {
        const auto res = *this;
        std::apply([this](auto&... args) { (++args, ...); }, this->_iterators);
        return res;
    }

    inline auto& operator--() noexcept(NO_EXCEPT) {
        std::apply([this](auto&... args) { (--args, ...); }, this->_iterators);
        return *this;
    }
    inline auto operator--(int) noexcept(NO_EXCEPT) {
        const auto res = *this;
        std::apply([this](auto&... args) { (--args, ...); }, this->_iterators);
        return res;
    }

    inline auto& operator+=(const difference_type diff) noexcept(NO_EXCEPT) {
        std::apply([this,diff](auto&&... args) { ((args += diff), ...); }, this->_iterators);
        return *this;
    }

    inline auto& operator-=(const difference_type diff) noexcept(NO_EXCEPT) {
        std::apply([this,diff](auto&... args) { ((args -= diff), ...); }, this->_iterators);
        return *this;
    }

    friend inline zip_iterator operator+(zip_iterator lhs, const difference_type rhs) noexcept(NO_EXCEPT) { return lhs += rhs; }
    friend inline zip_iterator operator-(zip_iterator lhs, const difference_type rhs) noexcept(NO_EXCEPT) { return lhs -= rhs; }

    friend inline difference_type operator-(const zip_iterator& lhs, const zip_iterator& rhs) noexcept(NO_EXCEPT) {
        return std::get<0>(lhs._iterators) - std::get<0>(rhs._iterators);
    }

    protected:
    template<std::size_t... I>
    const auto dereference_elements_impl(std::index_sequence<I...>) noexcept(NO_EXCEPT) { return std::tie(*std::get<I>(this->_iterators)...); }
    template<std::size_t... I>
    const auto dereference_elements_impl(std::index_sequence<I...>) const noexcept(NO_EXCEPT) { return std::tie(*std::get<I>(this->_iterators)...); }

    const auto dereference_elements() noexcept(NO_EXCEPT) { return dereference_elements_impl(indices()); }
    const auto dereference_elements() const noexcept(NO_EXCEPT) { return dereference_elements_impl(indices()); }

    public:
    const auto operator*() noexcept(NO_EXCEPT) { return dereference_elements(); }
    const auto operator*() const noexcept(NO_EXCEPT) { return dereference_elements(); }

    // For range base for.
    friend inline bool operator==(const zip_iterator &lhs, const zip_iterator &rhs) noexcept(NO_EXCEPT) { return lhs._iterators == rhs._iterators; }
    friend inline bool operator!=(const zip_iterator &lhs, const zip_iterator &rhs) noexcept(NO_EXCEPT) { return lhs._iterators != rhs._iterators; }
    friend inline bool operator<=(const zip_iterator &lhs, const zip_iterator &rhs) noexcept(NO_EXCEPT) { return lhs._iterators <= rhs._iterators; }
    friend inline bool operator>=(const zip_iterator &lhs, const zip_iterator &rhs) noexcept(NO_EXCEPT) { return lhs._iterators >= rhs._iterators; }
    friend inline bool operator<(const zip_iterator &lhs, const zip_iterator &rhs) noexcept(NO_EXCEPT) { return lhs._iterators < rhs._iterators; }
    friend inline bool operator>(const zip_iterator &lhs, const zip_iterator &rhs) noexcept(NO_EXCEPT) { return lhs._iterators > rhs._iterators; }
};

} // namespace view_impl

} // namespace internal



// Thanks to: https://qiita.com/pshiko/items/aee2641149b0cc97e287
template<class... Views>
struct zip_view : internal::view_impl::base {
  public:
    using iterator = internal::view_impl::zip_iterator<zip_view, internal::iterator_t<Views>...>;
    using const_iterator = internal::view_impl::zip_iterator<zip_view, internal::iterator_t<const Views>...>;

    using size_type = std::common_type_t<typename Views::size_type...>;
    using value_type = std::common_type_t<typename iterator::value_type>;

  protected:
    std::tuple<Views...> _bases;
    iterator _begin, _end;

  public:
    explicit zip_view(const Views... views) noexcept(NO_EXCEPT) : _bases({ views... }) {
        std::apply(
            [this](const auto&... args) {
                this->_begin = iterator{ std::begin(args)... };
                this->_end = iterator{ std::end(args)... };
            },
            _bases
        );
    };

    iterator begin() noexcept(NO_EXCEPT) { return this->_begin; }
    iterator end() noexcept(NO_EXCEPT) { return this->_end; }

    const_iterator begin() const noexcept(NO_EXCEPT) { return this->_begin.as_const(); }
    const_iterator end() const noexcept(NO_EXCEPT) { return this->_end.as_const(); }

    const_iterator cbegin() const noexcept(NO_EXCEPT) { return this->_begin.as_const(); }
    const_iterator cend() const noexcept(NO_EXCEPT) { return this->_end.as_const(); }
};


namespace views {


template<class... Views>
inline auto zip(Views&&... views) noexcept(NO_EXCEPT) { return zip_view(range(views)...); };


} // namespace views


} // namespace lib


#endif
