#pragma once

#include "../types.h"

struct RenderParameters
{
    u32 max_quads_per_frame;
};

struct Vertex
{
    v3 position;
};

struct RenderState
{
    v2u draw_region;
    
    u32 max_vertex_count;
    u32 vertex_count;
    Vertex* vertices;

    u32 max_index_count;
    u32 index_count;
    u16* indices;
};

struct RenderAPI
{
    void* platform;
};

#define RENDER_BEGIN_FRAME(name) RenderState* name(RenderAPI* render_api, v2u draw_space)
typedef RENDER_BEGIN_FRAME(render_begin_frame_t);

#define RENDER_END_FRAME(name) void name(RenderAPI* render_api, RenderState* render_state)
typedef RENDER_END_FRAME(render_end_frame_t);

void push_quad(v3 p0, v3 p1, v3 p2, v3 p3, RenderState* render_state);