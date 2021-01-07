#pragma once

#include <stdint.h>

union v2
{
    float E[2];
    struct
    {
        float x, y;
    };
};

union v2u
{
    uint32_t E[2];
    struct 
    {
        uint32_t x, y;
    };
};

#if COMPILER_MSVC
inline uint32_t AtomicCompareExchangeU32(uint32_t volatile *value, uint32_t new_value, uint32_t expected)
{
    return _InterlockedCompareExchange((long volatile *)value, new_value, expected);
}
#endif