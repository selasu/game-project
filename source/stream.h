#pragma once

#include "types.h"
#include "macro_util.h"

struct StreamChunk 
{
    void* data;
    u32 data_size;

    StreamChunk* next;
};

struct StreamBuffer
{
    void* data;
    u32 data_size;

    u32 bit_count;
    u32 bit_buffer;

    bool underflowed;

    StreamChunk* first;
    StreamChunk* last;
};

#define consume(buffer, type) (type*)consume_size(buffer, sizeof(type));
void* consume_size(StreamBuffer* buffer, u32 size);
u32 peek_bits(StreamBuffer* buffer, u32 bit_count);
void discard_bits(StreamBuffer* buffer, u32 bit_count);
u32 consume_bits(StreamBuffer* buffer, u32 bit_count);
void flush_byte(StreamBuffer* buffer);