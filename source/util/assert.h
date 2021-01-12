#pragma once

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