#pragma once

#include <stdint.h>

#define WAVE_FORMAT_PCM        0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW       0x0006
#define WAVE_FORMAT_MULAW      0x0007
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE 	

namespace engine
{
    void load_wav(const char* file_name);
} // engine