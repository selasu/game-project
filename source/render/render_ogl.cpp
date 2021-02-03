#include "render_ogl.h"
#include "../assert.h"

#define acount(a) (sizeof(a) / sizeof((a)[0]))

#define GL_ELEMENT_ARRAY_BUFFER  0x8893
#define GL_ARRAY_BUFFER          0x8892
#define GL_STATIC_DRAW           0x88E4
#define GL_STREAM_DRAW           0x88E0

#define GL_FRAGMENT_SHADER  0x8B30
#define GL_VERTEX_SHADER    0x8B31
#define GL_COMPILE_STATUS   0x8B81
#define GL_LINK_STATUS      0x8B82
#define GL_VALIDATE_STATUS  0x8B83

char* vertex_source = R"FOO(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    void main()
    {
       gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
    )FOO";

char* fragment_source = R"FOO(
    #version 330 core
    out vec4 FragColor;
    void main()
    {
       FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    )FOO";

GLuint opengl_create_program(OpenGL* opengl, char* vertex_code, char* fragment_code)
{
    GLuint vertex_id = opengl->glCreateShader(GL_VERTEX_SHADER);
    GLchar* vertex_shader_code[] = 
    {
        vertex_code
    };
    opengl->glShaderSource(vertex_id, acount(vertex_shader_code), vertex_shader_code, 0);
    opengl->glCompileShader(vertex_id);
    
    GLuint fragment_id = opengl->glCreateShader(GL_FRAGMENT_SHADER);
    GLchar* fragment_shader_code[] =
    {
        fragment_code
    };
    opengl->glShaderSource(fragment_id, acount(fragment_shader_code), fragment_shader_code, 0);
    opengl->glCompileShader(fragment_id);

    GLuint program_id = opengl->glCreateProgram();
    opengl->glAttachShader(program_id, vertex_id);
    opengl->glAttachShader(program_id, fragment_id);
    opengl->glLinkProgram(program_id);

    opengl->glValidateProgram(program_id);
    GLint linked = false;
    opengl->glGetProgramiv(program_id, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        // NOTE(selina): These error buffers exist solely to inspect during debugging
        GLsizei ignored;
        char vertex_error[1024];
        char fragment_error[1024];
        char program_error[1024];

        opengl->glGetShaderInfoLog(vertex_id, sizeof(vertex_error), &ignored, vertex_error);
        opengl->glGetShaderInfoLog(fragment_id, sizeof(fragment_error), &ignored, fragment_error);
        opengl->glGetProgramInfoLog(program_id, sizeof(program_error), &ignored, program_error);

        ASSERT(!"Shader validation failed");
    }

    return program_id;
}

RenderState* opengl_begin_frame(OpenGL* opengl, v2u draw_space)
{
    RenderState* render_state = &opengl->render_state;

    render_state->draw_region = draw_space;

    render_state->indices = opengl->indices;
    render_state->index_count = 0;
    render_state->max_index_count = opengl->max_index_count;

    render_state->vertices = opengl->vertices;
    render_state->vertex_count = 0;
    render_state->max_vertex_count = opengl->max_vertex_count;

    return render_state;
}

void opengl_end_frame(OpenGL* opengl, RenderState* render_state)
{
    opengl->glUseProgram(opengl->program);
    
    opengl->glBindVertexArray(opengl->vao);

    opengl->glBindBuffer(GL_ARRAY_BUFFER, opengl->vbo);
    opengl->glBufferData(GL_ARRAY_BUFFER, render_state->vertex_count * sizeof(Vertex), render_state->vertices, GL_STREAM_DRAW);
    
    opengl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl->ebo);
    opengl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, render_state->index_count * sizeof(u16), render_state->indices, GL_STREAM_DRAW);

    opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    opengl->glEnableVertexAttribArray(0);

    opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);

    glViewport(0, 0, render_state->draw_region.width, render_state->draw_region.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glDrawElements(GL_TRIANGLES, render_state->index_count * sizeof(u16), GL_UNSIGNED_SHORT, 0);
}

void init_opengl(OpenGL* opengl)
{
    opengl->glGenVertexArrays(1, &opengl->vao);
    opengl->glGenBuffers(1, &opengl->vbo);
    opengl->glGenBuffers(1, &opengl->ebo);

    opengl->program = opengl_create_program(opengl, vertex_source, fragment_source);
}