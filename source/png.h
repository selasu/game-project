#pragma once

#include "types.h"
#include "platform/platform.h"

struct LoadedPNG
{
    u32* data;
    u32  width;
    u32  height;
};

LoadedPNG load_png(FileContent file_content);