/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ GCC */

#define PROBLEM "https://atcoder.jp/contests/abc279/tasks/abc279_d"
#define ERROR 1E-12

#include <iostream>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "adapter/io.hpp"
#include "numeric/extremum_seeker.hpp"

signed main() {
    lib::i64 a, b; std::cin >> a >> b;

    auto f = [&](lib::i64 t) -> lib::ld {
        return a / std::sqrt(t + 1) + 1.0 * b * t;
    };

    lib::extremum_seeker<lib::ld,lib::i64> seeker(f);
    // debug(seeker.arg_min());
    print(seeker.min());
    return 0;
}
