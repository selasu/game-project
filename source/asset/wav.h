#pragma once

#include <stdint.h>

struct LoadedSound
{
    int32_t sample_count;
    int32_t channel_count;

    int16_t* samples;
};

LoadedSound load_wav(void* data);