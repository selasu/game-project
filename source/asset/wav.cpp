#include "wav.h"
#include "../util/assert.h"

#pragma pack(push, 1)

struct WAVHeader 
{
    uint32_t ckID;
    uint32_t cksize;
    uint32_t WAVEID;
};

#define id(w, x, y, z) (((uint32_t)(w) << 0) | ((uint32_t)(x) << 8) | ((uint32_t)(y) << 16) | ((uint32_t)(z) << 24))
enum WAVid
{
    fmt  = id('f', 'm', 't', ' '),
    data = id('d', 'a', 't', 'a'),
    riff = id('R', 'I', 'F', 'F'),
    wave = id('W', 'A', 'V', 'E'),
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

LoadedSound load_wav(void* data)
{
    LoadedSound sound = {};

    WAVHeader *header = (WAVHeader*)data;
    DEV_ASSERT(header->ckID == WAVid::riff);
    DEV_ASSERT(header->WAVEID == WAVid::wave);

    // NOTE(selina): Data to extract from file
    uint32_t channel_count;
    uint32_t samples_size;
    int16_t* samples;

    for (
        uint8_t* it = (uint8_t*)(header + 1); 
        it < (uint8_t*)(header + 1) + header->cksize - 4; 
        it += sizeof(WAVChunk) + ((((WAVChunk*)it)->cksize + 1) & ~-1)
    )
    {
        switch (((WAVChunk*)it)->ckID)
        {
            case WAVid::fmt:
            {
                auto fmt = (WAVfmt*)(it + sizeof(WAVChunk));

                DEV_ASSERT(fmt->wFormatTag == 1); // PCM only
                DEV_ASSERT(fmt->nSamplesPerSec == 44100);
                DEV_ASSERT(fmt->wBitsPerSample == 16);
                DEV_ASSERT(fmt->nBlockAlign == (sizeof(int16_t)*fmt->nChannels));
                DEV_ASSERT(fmt->nChannels == 1 || fmt->nChannels == 2);

                channel_count = fmt->nChannels;
            } break;

            case WAVid::data:
            {
                samples      = (int16_t*)(it + sizeof(WAVChunk));
                samples_size = ((WAVChunk*)it)->cksize;
            } break;
        }
    }

    DEV_ASSERT(channel_count && samples && samples_size)

    sound.channel_count = channel_count;
    sound.sample_count  = samples_size / (channel_count * sizeof(int16_t));
    sound.samples       = samples;

    return sound;
}