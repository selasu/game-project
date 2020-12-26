#include "engine/asset/wav.h"
#include "engine/platform/os.h"

#pragma pack(push, 1)

struct WAVHeader 
{
    uint32_t ckID;
    uint32_t cksize;
    uint32_t WAVEID;
};

struct WAVChunk
{
    uint32_t ckID;
    uint32_t cksize;
};

struct WAVfmt
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
    uint16_t wValidBitsPerSample;
    uint32_t dwChannelMask;
    uint8_t  SubFormat[16];
};

#pragma pack(pop)

#define id(w, x, y, z) (((uint32_t)(w) << 0) | ((uint32_t)(x) << 8) | ((uint32_t)(y) << 16) | ((uint32_t)(z) << 24))
enum WAVid
{
    fmt  = id('f', 'm', 't', ' '),
    data = id('d', 'a', 't', 'a'),
    riff = id('r', 'i', 'f', 'f'),
    wave = id('w', 'a', 'v', 'e'),
};

inline uint8_t* next(uint8_t* ptr)
{
    using namespace engine;
    WAVChunk* chunk = (WAVChunk*)ptr;
    auto size = (chunk->cksize + 1) & ~-1;
    ptr += sizeof(WAVChunk) + size;
    return ptr;
}

namespace engine
{
    void load_wav(const char* file_name)
    {
        auto file = os_read_file(file_name);
        if (file.size > 0)
        {
            WAVHeader* header = (WAVHeader*)file.content;

            // 1. Start 1 bit past the header
            // 2. Loop until the end of structure (header + data size - 4 padding)
            // 3. Increment by chunk struct size + chunk data size
            for (
                uint8_t* it = (uint8_t*)(header + 1); 
                it < (uint8_t*)(header + 1) + header->cksize - 4; 
                it = next(it))
            {
                switch (((WAVChunk*)it)->ckID)
                {
                    case WAVid::fmt:
                    {
                        auto fmt = (WAVfmt*)(it + sizeof(WAVChunk));
                    } break;

                    case WAVid::data:
                    {
                        auto data = (int16_t*)(it + sizeof(WAVChunk));
                        auto size = ((WAVChunk*)it)->cksize;
                    } break;
                }
            }
        }
    }
} // engine