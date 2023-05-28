#pragma once


#include <string>
#include <iterator>
#include <numeric>

#include "internal/types.hpp"


namespace lib {


template<class I>
std::string join(const I first, const I last, const std::string& sep = "") noexcept(NO_EXCEPT) {
    std::ostringstream res;
    std::copy(first, last, std::ostream_iterator<typename std::iterator_traits<I>::value_type>(res, sep));
    return res.str();
}

template<class V>
std::string join(V& v, const std::string& sep = "") noexcept(NO_EXCEPT) {
    return join(std::begin(v), std::end(v), sep);
}


template<class I, class T = typename std::iterator_traits<I>::value_type>
T sum(const I first, const I second, const T& base = 0) noexcept(NO_EXCEPT) {
    return std::accumulate(first, second, base);
}

template<class V, class T = typename V::value_type>
auto sum(V& v, T base = 0) noexcept(NO_EXCEPT) {
    return sum(std::begin(v), std::end(v), base);
}


} // namespace lib
