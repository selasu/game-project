#include "render_ogl.h"
#include "../../util/assert.h"

#define acount(a) (sizeof(a) / sizeof((a)[0]))

#define GL_ELEMENT_ARRAY_BUFFER  0x8893
#define GL_ARRAY_BUFFER          0x8892
#define GL_STATIC_DRAW           0x88E4

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

constexpr float vertices[] = {
    0.5f,  0.5f, 0.0f,  // top right
    0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f  // top left
};

constexpr unsigned int indices[] = { 
    0, 1, 3,  // first triangle
    1, 2, 3   // second triangle
};

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

        DEV_ASSERTM(false, "Shader validation failed")
    }

    return program_id;
}

void opengl_end_frame(OpenGL* opengl)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    opengl->glUseProgram(opengl->program);
    
    opengl->glBindVertexArray(opengl->vao);

    opengl->glBindBuffer(GL_ARRAY_BUFFER, opengl->vbo);
    opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    opengl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl->ebo);
    opengl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    opengl->glEnableVertexAttribArray(0);

    opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void init_opengl(OpenGL* opengl)
{
    opengl->glGenVertexArrays(1, &opengl->vao);
    opengl->glGenBuffers(1, &opengl->vbo);
    opengl->glGenBuffers(1, &opengl->ebo);

    opengl->program = opengl_create_program(opengl, vertex_source, fragment_source);
}