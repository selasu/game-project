#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include "render.h"

#define WIN32_LOAD_RENDERER(name) void name(HDC device_context)
typedef WIN32_LOAD_RENDERER(win32_load_renderer_t);

static char* win32_render_functions[] =
{
    "win32_load_renderer",
    "win32_begin_frame",
    "win32_end_frame"
};

struct Win32RenderCode
{
    win32_load_renderer_t* win32_load_renderer;
    render_begin_frame_t*  win32_begin_frame;
    render_end_frame_t*    win32_end_frame;
};

#endif