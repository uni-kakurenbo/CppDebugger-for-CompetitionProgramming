/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ GCC */

#define PROBLEM "https://atcoder.jp/contests/abc242/tasks/abc242_g"

#include <iostream>
#include "snippet/aliases.hpp"
#include "snippet/fast_io.hpp"
#include "adapter/io.hpp"
#include "adapter/valarray.hpp"
#include "adapter/vector.hpp"
#include "iterable/operation.hpp"
#include "numeric/interval_scanner.hpp"

signed main() {
    int n; std::cin >> n;
    lib::valarray<lib::i64> a(n); input >> a; a -= 1;

    int q; std::cin >> q;
    lib::valarray<lib::spair<int>> qs(q); input >> qs;
    REP(i, q) qs[i].first--;

    lib::valarray<lib::i64> cnt(n);
    lib::i64 ans = 0;

    auto expand = [&](int p) {
        int v = a[p];
        cnt[v]++;
        if(cnt[v]%2 == 0) ans++;
    };
    auto contract = [&](int p) {
        int v = a[p];
        cnt[v]--;
        if(cnt[v]%2 == 1) ans--;
    };
    auto eval = [&]() { return ans; };

    lib::interval_plannner planner(expand, contract, eval);
    print(lib::join(planner.scan(qs), "\n"));
}
