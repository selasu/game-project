#include "engine/asset/wav.h"

#include "engine/platform/os.h"
#include "engine/util/assert.h"

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
    riff = id('R', 'I', 'F', 'F'),
    wave = id('W', 'A', 'V', 'E'),
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
    Sound load_wav(const char* file_name)
    {
        Sound sound;

        auto file = os_read_file(file_name);
        if (file.size > 0)
        {
            WAVHeader* header = (WAVHeader*)file.content;
            DEV_ASSERT(header->ckID == WAVid::riff);
            DEV_ASSERT(header->WAVEID == WAVid::wave);

            // Data to extract from file
            uint32_t channel_count = 0;
            uint32_t samples_size  = 0;
            int16_t* samples       = nullptr;

            // 1. Start 1 bit past the header
            // 2. Loop until the end of structure (header + data size - 4 padding)
            // 3. Increment by chunk struct size + chunk data size
            for (uint8_t* it = (uint8_t*)(header + 1); it < (uint8_t*)(header + 1) + header->cksize - 4; it = next(it))
            {
                switch (((WAVChunk*)it)->ckID)
                {
                    case WAVid::fmt:
                    {
                        auto fmt = (WAVfmt*)(it + sizeof(WAVChunk));

                        DEV_ASSERT(fmt->wFormatTag == 1); // PCM only
                        DEV_ASSERT(fmt->wBitsPerSample == 16);
                        DEV_ASSERT(fmt->nBlockAlign == (sizeof(int16_t)*fmt->nChannels));

                        channel_count = fmt->nChannels;
                    } break;

                    case WAVid::data:
                    {
                        samples      = (int16_t*)(it + sizeof(WAVChunk));
                        samples_size = ((WAVChunk*)it)->cksize;
                    } break;
                }
            }

            DEV_ASSERT(channel_count && samples);

            uint32_t sample_count = samples_size / (channel_count * sizeof(int16_t));
            sound.sample_count = sample_count;
            sound.channel_count = channel_count;
            sound.samples = (int16_t*)os_get_memory(sound.channel_count * sound.sample_count * sizeof(int16_t));

            int16_t* source = samples;
            int16_t* target = 0;
            for (uint32_t sindex = 0; sindex < sample_count; ++sindex)
            {
                for (uint32_t cindex = 0; cindex < channel_count; ++cindex)
                {
                    target = sound.samples + (cindex * sound.sample_count);
                    target[sindex] = *source++;
                }
            }
        }

        return sound;
    }
} // engine