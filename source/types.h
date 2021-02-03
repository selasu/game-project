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
    f32 E[2];
    struct
    {
        f32 x, y;
    };
};

union v2u
{
    u32 E[2];
    struct 
    {
        u32 x, y;
    };
    struct
    {
        u32 width, height;
    };
};

union v3
{
    f32 E[3];
    struct
    {
        f32 x, y, z;
    };
};

union v3u
{
    u32 E[3];
    struct
    {
        u32 x, y, z;
    };
};

union v4
{
    f32 E[4];
    struct
    {
        f32 w, x, y, z;
    };
    struct
    {
        f32 r, g, b, a;
    };
};

union v4u
{
    u32 E[4];
    struct
    {
        u32 w, x, y, z;
    };
    struct
    {
        u32 r, g, b, a;
    };
};

#if COMPILER_MSVC
inline u32 AtomicCompareExchangeU32(u32 volatile *value, u32 new_value, u32 expected)
{
    return _InterlockedCompareExchange((long volatile *)value, new_value, expected);
}
#endif