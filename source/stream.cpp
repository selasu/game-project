#include "stream.h"

void* consume_size(StreamBuffer* buffer, u32 size)
{
    void* data = 0;

    if (buffer->data_size == 0 && buffer->first)
    {
        StreamChunk* chunk = buffer->first;
        buffer->data_size = chunk->data_size;
        buffer->data = chunk->data;
        buffer->first = chunk->next;
    }

    if (buffer->data_size >= size)
    {
        data = buffer->data;
        buffer->data = (u8*)buffer->data + size;
        buffer->data_size -= size;
    }
    else
    {
        buffer->underflowed = true;
        buffer->data_size = 0;
    }

    return data;
}

u32 peek_bits(StreamBuffer* buffer, u32 bit_count)
{
    ASSERT(bit_count <= 32);

    while (buffer->bit_count < bit_count && !buffer->underflowed)
    {
        u32 byte = *consume(buffer, u8);
        buffer->bit_buffer |= (byte << buffer->bit_count);
        buffer->bit_count += 8;
    }

    u32 bits = buffer->bit_buffer & ((1 << bit_count) - 1);
    buffer->underflowed |= buffer->bit_count < bit_count;

    return bits;
}

void discard_bits(StreamBuffer* buffer, u32 bit_count)
{
    buffer->bit_count -= bit_count;
    buffer->bit_buffer >>= bit_count;
}

u32 consume_bits(StreamBuffer* buffer, u32 bit_count)
{
    u32 bits = peek_bits(buffer, bit_count);
    discard_bits(buffer, bit_count);

    return bits;
}

void flush_byte(StreamBuffer* buffer)
{
    buffer->bit_buffer = 0;
    buffer->bit_count = 0;
}