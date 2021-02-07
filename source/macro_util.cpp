#include "macro_util.h"

void zero_size(size_t size, void* ptr)
{
    u8* byte = (u8*)ptr;
    while (--size) *byte++ = 0;
}