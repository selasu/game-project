#pragma once

#include "types.h"

struct LoadedSound
{
    i32 sample_count;
    i32 channel_count;

    i16* samples;
};

LoadedSound load_wav(void* data);