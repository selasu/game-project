#pragma once

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

union v2
{
    float E[2];
    struct
    {
        f32 x, y;
    };
};

union v2u
{
    uint32_t E[2];
    struct 
    {
        u32 x, y;
    };
    struct
    {
        u32 width, height;
    };
};

#if COMPILER_MSVC
inline u32 AtomicCompareExchangeU32(u32 volatile *value, u32 new_value, u32 expected)
{
    return _InterlockedCompareExchange((long volatile *)value, new_value, expected);
}
#endif