#pragma once

#include "types.h"

#ifdef ASSERTIONS_ON
    #define ASSERT(expr) if (!(expr)) *(volatile int*)0 = 0

    #ifndef NDEBUG
        #define DEBUG_ASSERT(expr) DEV_ASSERT(expr)
    #else
        #define DEBUG_ASSERT(expr)
    #endif
#else
    #define ASSERT(expr)
    #define DEBUG_ASSERT(expr)
#endif

#define array_count(arr) ((sizeof(arr) / sizeof((arr)[0])))
#define zero_array(size, ptr) zero_size((size) * sizeof((ptr)[0]), (ptr))
void zero_size(size_t size, void* ptr);