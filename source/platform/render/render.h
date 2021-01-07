#pragma once

struct RenderAPI
{
    void* platform;
};

#define RENDER_BEGIN_FRAME(name) void name()
typedef RENDER_BEGIN_FRAME(render_begin_frame_t);

#define RENDER_END_FRAME(name) void name()
typedef RENDER_END_FRAME(render_end_frame_t);