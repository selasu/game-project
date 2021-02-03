#include "render.h"
#include "../assert.h"

void push_quad(v3 p0, v3 p1, v3 p2, v3 p3, RenderState* render_state)
{
    u32 vertex_index = render_state->vertex_count;
    u32 index_index  = render_state->index_count;

    render_state->vertex_count += 4;
    render_state->index_count += 6;

    ASSERT(render_state->vertex_count <= render_state->max_vertex_count);
    ASSERT(render_state->index_count <= render_state->max_index_count);

    Vertex* vertex = render_state->vertices + vertex_index;
    u16* index = render_state->indices + index_index;

    vertex[0].position = p0;
    vertex[1].position = p1;
    vertex[2].position = p2;
    vertex[3].position = p3;
    
    index[0] = vertex_index + 0;
    index[1] = vertex_index + 1;
    index[2] = vertex_index + 3;
    index[3] = vertex_index + 1;
    index[4] = vertex_index + 2;
    index[5] = vertex_index + 3;
}