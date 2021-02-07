#include "png.h"
#include "macro_util.h"
#include "stream.h"

#include <stdio.h>
#include <stdlib.h>

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

struct PNGHuffmanEntry
{
    u16 symbol;
    u16 bits_used;
};

struct PNGHuffman
{
    PNGHuffmanEntry* entries;
    u32 entry_count;
};

#define FourCC(str) (((u32)(str[0]) << 0) | ((u32)(str[1]) << 8) | ((u32)(str[2]) << 16) | ((u32)(str[3]) << 24))

static constexpr u8 png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
static constexpr u32 HCLEN_translation_table[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

void flip_endian(u32* value)
{
    u32 v = *value;

    *value = (v << 24) |
            ((v & 0xFF00) << 8) |
            ((v >> 8) & 0xFF00) |
            (v >> 24);
}

PNGHuffman allocate_huffman(u32 max_code_length)
{
    ASSERT(max_code_length <= 16);

    PNGHuffman huffman = {};

    huffman.entry_count = 1 << max_code_length;
    huffman.entries = (PNGHuffmanEntry*)malloc(sizeof(PNGHuffmanEntry) * huffman.entry_count);

    return huffman;
}

u32 decode_huffman(PNGHuffman* huffman, StreamBuffer* input)
{
    u32 result = 0;

    return result;
}

void create_huffman(u32 input_size, u32* input, PNGHuffman* output)
{

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
                        fprintf(stdout, "Chunk: IDAT (%u)\n", chunk_header->length);

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
            // NOTE(selina): zlib compression spec here: https://www.ietf.org/rfc/rfc1951.txt

            PNGidatHeader* idat_header = consume(&compressed_data, PNGidatHeader);

            u8 cm =  idat_header->zlib_flags & 0xF;
            u8 cinfo = idat_header->zlib_flags >> 4;
            u8 fcheck = idat_header->additional_flags & 0x1F;
            u8 fdict = (idat_header->additional_flags >> 5) & 0x1;
            u8 flevel = idat_header->additional_flags >> 6;

            supported = cm == 8 && fdict == 0;

            if (supported)
            {
                fprintf(stdout, "Supported: {%u, %u}\n", width, height);

                u32 BFINAL = 0;
                while (BFINAL == 0)
                {
                    BFINAL = consume_bits(&compressed_data, 1);
                    u32 BTYPE = consume_bits(&compressed_data, 1);

                    if (BTYPE == 0)
                    {
                        flush_byte(&compressed_data);

                        u32 LEN = consume_bits(&compressed_data, 16);
                        i32 NLEN = consume_bits(&compressed_data, 16);
                        if (LEN != -NLEN)
                        {
                            fprintf(stderr, "Oops\n");
                        }
                    }
                    else if (BTYPE == 3)
                    {
                        // NOTE(selina): This is bad
                    }
                    else
                    {
                        PNGHuffman literal_len_huffman;
                        PNGHuffman distance_huffman;

                        if (BTYPE == 2)
                        {
                            u32 HLIT = consume_bits(&compressed_data, 5);
                            u32 HDIST = consume_bits(&compressed_data, 5);
                            u32 HCLEN = consume_bits(&compressed_data, 4);

                            HLIT += 257;
                            HDIST += 1;
                            HCLEN += 4;
                            ASSERT(HCLEN <= array_count(HCLEN_translation_table));

                            u32 HCLEN_code_table[array_count(HCLEN_translation_table)] = {};
                            for (u32 index = 0; index < HCLEN; ++index)
                            {
                                HCLEN_code_table[HCLEN_translation_table[index]] = consume_bits(&compressed_data, 3);
                            }
                            
                            PNGHuffman dict_huffman;
                            create_huffman(array_count(HCLEN_translation_table), HCLEN_code_table, &dict_huffman);

                            u32 huffman_codes[512];
                            u32 huffman_codes_count = 0;
                            while (huffman_codes_count < HLIT + HCLEN)
                            {
                                u32 repeat_count = 1;
                                u32 repeat_value = 0;
                                u32 encoded_value = decode_huffman(&dict_huffman, &compressed_data);

                                if (encoded_value <= 15)
                                {
                                    repeat_value = encoded_value;
                                }
                                else if (encoded_value == 16)
                                {
                                    ASSERT(huffman_codes_count > 0);

                                    repeat_count = 3 + consume_bits(&compressed_data, 2);
                                    repeat_value = huffman_codes[huffman_codes_count - 1];
                                }
                                else if (encoded_value == 17)
                                {
                                    repeat_count = 3 + consume_bits(&compressed_data, 2);
                                }
                                else if (encoded_value == 18)
                                {
                                    repeat_count = 11 + consume_bits(&compressed_data, 7);
                                }
                                else
                                {
                                    // NOTE(selina): This is bad
                                }

                                while (repeat_count--)
                                {
                                    huffman_codes[huffman_codes_count++] = repeat_value;
                                }
                            }

                            ASSERT(huffman_codes_count == HLIT + HCLEN);
                        }
                    }
                }
            }
        }
    }

    return png;
}