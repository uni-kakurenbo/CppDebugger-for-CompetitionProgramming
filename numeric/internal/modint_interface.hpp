#pragma once


#include <concepts>
#include <type_traits>
#include <atcoder/modint>


#include "snippet/aliases.hpp"

#include "internal/concepts.hpp"


namespace lib {


namespace internal {


template<std::unsigned_integral Value, std::unsigned_integral Large, Value Mod>
    requires
        (std::numeric_limits<Value>::digits <= 64) &&
        (2 * std::numeric_limits<Value>::digits <= std::numeric_limits<Large>::digits) &&
        (0 < Mod)
struct static_modint_impl;

template<std::unsigned_integral Value, std::unsigned_integral Large, i64 Id>
    requires
        (std::numeric_limits<Value>::digits <= 64) &&
        (2 * std::numeric_limits<Value>::digits <= std::numeric_limits<Large>::digits)
struct dynamic_modint_impl;


using atcoder::internal::is_modint;
template<class T> constexpr bool is_modint_v = is_modint<T>::value;
using atcoder::internal::is_modint_t;

template<class T> concept modint_family =
    numeric<T> &&
    requires (T v, i64 p) {
        { v.pow(p) } -> std::same_as<T>;
        { v.inv() } -> std::same_as<T>;

        { v.val() } -> std::same_as<typename T::unsigned_value_type>;
        { T::mod() } -> std::same_as<typename T::unsigned_value_type>;
        { T::max() } -> std::same_as<typename T::unsigned_value_type>;
        T::digits;
    };

template<class T>
concept dynamic_modint_family =
    modint_family<T> &&
    requires (int v) {
        T::set_mod(v);
    };

template<class T> concept static_modint_family = modint_family<T> && !dynamic_modint_family<T>;


} // namespace internal


template<u32 Mod> using static_modint_32bit = internal::static_modint_impl<u32, u64, Mod>;
template<u64 Mod> using static_modint_64bit = internal::static_modint_impl<u64, u128, Mod>;

template<i64 id = -1> using dynamic_modint_32bit = internal::dynamic_modint_impl<u32, u64, id>;
template<i64 id = -1> using dynamic_modint_64bit = internal::dynamic_modint_impl<u64, u128, id>;

template<u32 Mod> using static_modint = static_modint_32bit<Mod>;
template<i64 id = -1> using dynamic_modint = dynamic_modint_32bit<id>;

using modint998244353 = static_modint_32bit<998244353>;
using modint1000000007 = static_modint_32bit<1000000007>;

using modint = dynamic_modint_32bit<-1>;
using modint64 = dynamic_modint_64bit<-1>;


} // namespace lib
