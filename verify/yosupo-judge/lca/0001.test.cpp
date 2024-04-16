/*
 * @uni_kakurenbo
 * https://github.com/uni-kakurenbo/competitive-programming-workspace
 *
 * CC0 1.0  http://creativecommons.org/publicdomain/zero/1.0/deed.ja
 */
/* #language C++ GCC */

#define PROBLEM "https://judge.yosupo.jp/problem/lca"

#include "structure/graph.hpp"
#include "graph/lowest_common_ancestor.hpp"

signed main() {
    int n, q; std::cin >> n >> q;
    uni::graph<> G(n);

    REP(i, 1, n) {
        int p; std::cin >> p;
        G.add_edge_bidirectionally(i, p);
    }

    uni::lowest_common_ancestor lca(G, 0);

    LOOP(q) {
        int u, v; std::cin >> u >> v;
        print(lca(u, v));
    }

    return 0;
}
