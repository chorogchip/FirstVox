#pragma once

#include <cinttypes>

#include "Macros.h"

namespace vox::rand
{

    class LinearGenerator
    {
    private:
        uint64_t x;
    public:
        static constexpr uint64_t A = 48271U;
        static constexpr uint64_t B = 0U;
        static constexpr uint64_t M = 2147483647U;

        FORCE_INLINE LinearGenerator(): x(0) {}
        FORCE_INLINE LinearGenerator(uint32_t seed): x(seed) {}
        FORCE_INLINE void seed(uint32_t seed) { x = (uint64_t)seed; }
        FORCE_INLINE uint32_t Sample() { return (uint32_t)(x = (A * x + B) % M); }
        FORCE_INLINE uint32_t operator() () { return (uint32_t)(x = (A * x + B) % M); }
        FORCE_INLINE static constexpr uint32_t min() { return 0; }
        FORCE_INLINE static constexpr uint32_t max() { return (uint32_t)-1; }
    };

}