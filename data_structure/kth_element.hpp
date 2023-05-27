#pragma once

#include <vector>
#include <functional>
#include <queue>

#include "internal/dev_env.hpp"
#include "internal/types.hpp"

namespace lib {


template<class T, class container = std::vector<T>, class comparer = std::less<T>, class rev_comparer = std::greater<T>>
struct kth_element {
  protected:
    internal::size_t _k;
    std::priority_queue<T,container,comparer> small;
    std::priority_queue<T,container,rev_comparer> large;

  public:
    kth_element(internal::size_t k = 0) noexcept(DEV_ENV) : _k(k) {}

    inline T get() const noexcept(DEV_ENV) { return small.top(); }
    inline bool has() const noexcept(DEV_ENV) { return small.size() == _k; }

    inline void push(T v) noexcept(DEV_ENV) {
        if(small.size() < _k) {
            small.push(v);
            return;
        }
        T kth = small.top();
        if(v < kth) {
            small.pop(); small.push(v);
            large.push(kth);
        }
        else {
            large.push(v);
        }
    }

    inline void pop() noexcept(DEV_ENV) {
        small.pop();
        if(large.empty()) return;
        T v = large.top(); large.pop();
        small.push(v);
    }
};


} // namespace lib
