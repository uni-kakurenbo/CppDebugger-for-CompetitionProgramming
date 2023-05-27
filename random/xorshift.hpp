#pragma once


#include <cstdint>
#include <limits>

#include "internal/dev_env.hpp"


namespace lib {

struct xorshift {
    using result_type = std::uint32_t;

    static constexpr result_type MIN = std::numeric_limits<result_type>::min();
    static constexpr result_type MAX = std::numeric_limits<result_type>::max();

    static constexpr result_type min() noexcept(DEV_ENV) { return MIN; }
    static constexpr result_type max() noexcept(DEV_ENV) { return MAX; }

    inline void seed(unsigned int seed) noexcept(DEV_ENV) { this->w = seed; }

    constexpr xorshift() {};
    constexpr xorshift(const std::uint32_t seed) noexcept(DEV_ENV) : w(seed) {};

    inline std::uint32_t operator()() noexcept(DEV_ENV) {
        std::uint32_t t;

        t = x ^ (x << 11);
        x = y; y = z; z = w;
        return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
    }

  private:
    std::uint32_t x = 123456789;
    std::uint32_t y = 362436069;
    std::uint32_t z = 521288629;
    std::uint32_t w = 88675123;
};

struct xorshift64 {
    using result_type = std::uint64_t;

    static constexpr result_type MIN = std::numeric_limits<result_type>::min();
    static constexpr result_type MAX = std::numeric_limits<result_type>::max();

    static constexpr result_type min() noexcept(DEV_ENV) { return MIN; }
    static constexpr result_type max() noexcept(DEV_ENV) { return MAX; }

    inline void seed(unsigned int seed) noexcept(DEV_ENV) { this->x = seed; }

    constexpr xorshift64() noexcept(DEV_ENV) {};
    constexpr xorshift64(const std::uint64_t seed) noexcept(DEV_ENV) : x(seed) {};

    inline std::uint64_t operator()() noexcept(DEV_ENV) {
        x = x ^ (x << 13), x = x ^ (x <<  7), x = x ^ (x << 17);
        return x;
    }

  private:
    std::uint64_t x = 3141592653589793238UL;
};

xorshift rand;
xorshift64 rand64;


} // namespace lib
