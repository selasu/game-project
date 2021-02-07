#include "wav.h"
#include "macro_util.h"

#pragma pack(push, 1)

struct WAVHeader 
{
    u32 ckID;
    u32 cksize;
    u32 WAVEID;
};

#define id(w, x, y, z) (((u32)(w) << 0) | ((u32)(x) << 8) | ((u32)(y) << 16) | ((u32)(z) << 24))
enum WAVid
{
    fmt  = id('f', 'm', 't', ' '),
    data = id('d', 'a', 't', 'a'),
    riff = id('R', 'I', 'F', 'F'),
    wave = id('W', 'A', 'V', 'E'),
};

struct WAVChunk
{
    u32 ckID;
    u32 cksize;
};

struct WAVfmt
{
    u16 wFormatTag;
    u16 nChannels;
    u32 nSamplesPerSec;
    u32 nAvgBytesPerSec;
    u16 nBlockAlign;
    u16 wBitsPerSample;
    u16 cbSize;
    u16 wValidBitsPerSample;
    u32 dwChannelMask;
    u8  SubFormat[16];
};

#pragma pack(pop)

LoadedSound load_wav(void* data)
{
    LoadedSound sound = {};

    WAVHeader *header = (WAVHeader*)data;
    ASSERT(header->ckID == WAVid::riff);
    ASSERT(header->WAVEID == WAVid::wave);

    // NOTE(selina): Data to extract from file
    u32 channel_count;
    u32 samples_size;
    i16* samples;

    for (
        u8* it = (u8*)(header + 1); 
        it < (u8*)(header + 1) + header->cksize - 4; 
        it += sizeof(WAVChunk) + ((((WAVChunk*)it)->cksize + 1) & ~-1)
    )
    {
        switch (((WAVChunk*)it)->ckID)
        {
            case WAVid::fmt:
            {
                WAVfmt* fmt = (WAVfmt*)(it + sizeof(WAVChunk));

                ASSERT(fmt->wFormatTag == 1); // PCM only
                ASSERT(fmt->nSamplesPerSec == 44100);
                ASSERT(fmt->wBitsPerSample == 16);
                ASSERT(fmt->nBlockAlign == (sizeof(int16_t)*fmt->nChannels));
                ASSERT(fmt->nChannels == 1 || fmt->nChannels == 2);

                channel_count = fmt->nChannels;
            } break;

            case WAVid::data:
            {
                samples      = (int16_t*)(it + sizeof(WAVChunk));
                samples_size = ((WAVChunk*)it)->cksize;
            } break;
        }
    }

    ASSERT(channel_count && samples && samples_size);

    sound.channel_count = channel_count;
    sound.sample_count  = samples_size / (channel_count * sizeof(i16));
    sound.samples       = samples;

    return sound;
}