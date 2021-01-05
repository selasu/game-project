#pragma once

#include <stdint.h>

struct Sound
{
    int32_t sample_count;
    int32_t channel_count;

    int16_t* samples;
};

Sound load_wav(void* data);