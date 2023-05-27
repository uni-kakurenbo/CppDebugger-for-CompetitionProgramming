#pragma once


#include <cassert>
#include <cstddef>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

#include "internal/dev_env.hpp"


namespace lib {


template<std::size_t B = 2, class T>
std::string base_n_string(T v) noexcept(DEV_ENV) {
    constexpr char CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static_assert(0 < B and B <= std::strlen(CHARS));
    assert(0 <= v);

    std::string res;
    while(v > 0) {
        res += CHARS[v%B];
        v /= B;
    }
    std::reverse(ALL(res));

    return res;
}


template<class T>
std::string base_n_string(T v, std::size_t b) noexcept(DEV_ENV) {
    constexpr char CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    assert(1 < b && b <= std::strlen(CHARS));
    assert(0 <= v);

    std::string res;
    while(v > 0) {
        res += CHARS[v%b];
        v /= b;
    }
    std::reverse(ALL(res));

    return res;
}

template<class T>
std::vector<T> base_n_vector(T v, std::size_t b = 2) noexcept(DEV_ENV) {
    assert(1 < b);
    assert(0 <= v);

    std::vector<T> res;
    while(v > 0) {
        res.push_back(v%b);
        v /= b;
    }
    std::reverse(ALL(res));

    return res;
}


} // namespace lib
