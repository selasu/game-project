#pragma once

#ifdef ASSERTIONS_ON
    #include <stdio.h>

    #if _MSC_VER
        #include <intrin.h>
        #define DEBUG_BREAK __debugbreak()
    #else
        #define DEBUG_BREAK __asm { int 3 }
    #endif

    inline void report_assertion_failure(const char* expression, const char* message, const char* file, int line)
    {
        fprintf(stderr, "[%s; %d] %s (%s)\n", file, line, message, expression);
    }

    inline void report_assertion_failure(const char* expression, const char* file, int line)
    {
        fprintf(stderr, "[%s; %d] (%s)\n", file, line, expression);
    }

    #define DEV_ASSERT(expr) { \
        if (!(expr)) { \
            report_assertion_failure(#expr, __FILE__, __LINE__); \
            DEBUG_BREAK; \
        } \
    }

    #define DEV_ASSERTM(expr, msg) { \
        if (!(expr)) { \
            report_assertion_failure(#expr, msg, __FILE__, __LINE__); \
            DEBUG_BREAK; \
        } \
    }

    #ifndef NDEBUG
        #define DEV_DASSERT(expr) DEV_ASSERT(expr)
        #define DEV_DASSERTM(expr, msg) DEV_ASSERTM(expr, msg)
    #else
        #define DEV_DASSERT(expr) {}
        #define DEV_DASSERTM(expr, msg) {}
    #endif
#else
    #define DEV_ASSERT(expr) {}
    #define DEV_ASSERTM(expr, msg) {}
    #define DEV_DASSERT(expr) {}
    #define DEV_DASSERTM(expr, msg) {}
#endif