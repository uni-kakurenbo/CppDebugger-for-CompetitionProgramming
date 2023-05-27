#pragma once


#include <vector>
#include <queue>
#include <utility>
#include <functional>

#include "internal/dev_env.hpp"

#include "snippet/iterations.hpp"
#include "graph.hpp"


template<class edge_cost>
template<class cost_t>
void lib::graph<edge_cost>::distances(const size_type s, std::vector<cost_t> *const dists) const noexcept(DEV_ENV) {
    using state = std::pair<cost_t,size_type>;
    std::priority_queue<state,std::vector<state>,std::greater<state>> que;

    dists->assign(this->size(), std::numeric_limits<cost_t>::max());

    que.emplace(0, s), (*dists)[s] = 0;

    while(not que.empty()) {
        const auto [d, u] = que.top(); que.pop();

        if((*dists)[u] < d) continue;

        ITR(e, (*this)[u]) {
            const size_type v = e.to; const auto cost = e.cost;

            if((*dists)[v] <= d + cost) continue;

            (*dists)[v] = d + cost;
            que.emplace((*dists)[v], v);
        }
    }
}

template<class edge_cost>
template<class cost_t>
std::vector<cost_t> lib::graph<edge_cost>::distances(const size_type s) const noexcept(DEV_ENV) {
    std::vector<cost_t> dists;
    this->distances<cost_t>(s, &dists);
    return dists;
}
