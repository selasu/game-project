#include "png.h"
#include <stdio.h>

#pragma pack(push, 1)

struct PNGHeader
{
    u8 signature[8];
};

struct PNGChunkHeader
{
    u32 length;
    union
    {
        u32 chunk_type;
        char chunk_type_str[4];
    };
};

struct PNGChunkFooter
{
    u32 crc;
};

struct PNGihdr
{
    u32 width;
    u32 height;
    u8 bit_depth;
    u8 colour;
    u8 compression;
    u8 filter;
    u8 interlace;
};

struct PNGidatHeader
{
    u8 zlib_flags;
    u8 additional_flags;
};

#pragma pack(pop)

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

    StreamChunk* first;
    StreamChunk* last;
};

#define FourCC(str) (((u32)(str[0]) << 0) | ((u32)(str[1]) << 8) | ((u32)(str[2]) << 16) | ((u32)(str[3]) << 24))

static constexpr u8 png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

void flip_endian(u32* value)
{
    u32 v = *value;

    *value = (v << 24) |
            ((v & 0xFF00) << 8) |
            ((v >> 8) & 0xFF00) |
            (v >> 24);
}

#define consume(file, type) (type*)consume_size(file, sizeof(type));
void* consume_size(StreamBuffer* file, u32 size)
{
    void* data = 0;

    if (file->data_size == 0 && file->first)
    {
        StreamChunk* chunk = file->first;
        file->data_size = chunk->data_size;
        file->data = chunk->data;
        file->first = chunk->next;
    }

    if (file->data_size >= size)
    {
        data = file->data;
        file->data = (u8*)file->data + size;
        file->data_size -= size;
    }
    else
    {
        fprintf(stderr, "File underflowed at [%u] size.\n", size);
        file->data_size = 0;
    }

    return data;
}

LoadedPNG load_png(FileContent file_content)
{
    LoadedPNG png = {};

    StreamBuffer sb = {};
    sb.data = file_content.data;
    sb.data_size = file_content.file_size;

    StreamBuffer* at = &sb;

    bool supported = false;

    PNGHeader* header = consume(at, PNGHeader);
    if (header)
    {
        StreamBuffer compressed_data = {};

        u32 width = 0;
        u32 height = 0;

        while (at->data_size > 0)
        {
            PNGChunkHeader* chunk_header = consume(at, PNGChunkHeader);

            if (chunk_header)
            {
                flip_endian(&chunk_header->length);

                void* data = consume_size(at, chunk_header->length);
                PNGChunkFooter* chunk_footer = consume(at, PNGChunkFooter);

                switch(chunk_header->chunk_type)
                {
                    case FourCC("IHDR"):
                    {
                        fprintf(stdout, "Chunk: IHDR\n");

                        PNGihdr* ihdr = (PNGihdr*)data;
                        flip_endian(&ihdr->width);
                        flip_endian(&ihdr->height);

                        if (ihdr->bit_depth == 8 && ihdr->colour == 6 && ihdr->compression == 0 && ihdr->filter == 0 && ihdr->interlace == 0)
                        {
                            width = ihdr->width;
                            height = ihdr->height;
                            supported = true;
                        }
                    } break;
                    
                    case FourCC("IDAT"):
                    {
                        fprintf(stdout, "Chunk: IDAT\n");

                        StreamChunk* chunk = new StreamChunk;
                        chunk->data_size = chunk_header->length;
                        chunk->data = data;
                        chunk->next = 0;

                        compressed_data.last = (compressed_data.last ? compressed_data.last->next : compressed_data.first) = chunk;
                    } break;
                }
            }
        }

        if (supported)
        {
            fprintf(stdout, "Supported: {%u, %u}\n", width, height);
            // NOTE(selina): zlib compression spec here: https://www.ietf.org/rfc/rfc1950.txt

            PNGidatHeader* idat_header = consume(&compressed_data, PNGidatHeader);

            u8 cm =  idat_header->zlib_flags & 0xF;
            u8 cinfo = idat_header->zlib_flags >> 4;
            u8 fcheck = idat_header->additional_flags & 0x1F;
            u8 fdict = (idat_header->additional_flags >> 5) & 0x1;
            u8 flevel = idat_header->additional_flags >> 6;

            supported = cm == 8 && fdict == 0;

            if (supported)
            {

            }
        }
    }

    return png;
}