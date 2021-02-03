#pragma once

#include "render.h"

#if defined(_WIN32) || defined(_WIN64)
#  include <windows.h>
#  include <GL/gl.h>
#elif __APPLE__
#  error platform not supported.
#elif defined(__ANDROID__) || defined(ANDROID)
#  error platform not supported.
#elif defined(__linux__) || defined(__unix__) || defined(__posix__)
#  error platform not supported.
#else
#  error platform not supported.
#endif

typedef char      GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

typedef void type_glBindBuffer(GLenum target, GLuint buffer);
typedef void type_glBindVertexArray(GLuint array);
typedef void type_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void type_glGenBuffers(GLsizei n, GLuint* buffers);
typedef void type_glGenVertexArrays(GLsizei n, GLuint* arrays);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalised, GLsizei stride, const void* pointer);
typedef void type_glEnableVertexAttribArray(GLuint index);
typedef GLuint type_glCreateShader(GLenum shaderType);
typedef void type_glShaderSource(GLuint shader, GLsizei count, GLchar** string, const GLint* length);
typedef void type_glCompileShader(GLuint shader);
typedef GLuint type_glCreateProgram(void);
typedef void type_glAttachShader(GLuint program, GLuint shader);
typedef void type_glLinkProgram(GLuint program);
typedef void type_glValidateProgram(GLuint program);
typedef void type_glGetProgramiv(GLuint program, GLenum pname, GLint* params);
typedef void type_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
typedef void type_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
typedef void type_glDeleteShader(GLuint shader);
typedef void type_glUseProgram(GLuint program);

#define opengl_function(name) type_##name *name

struct OpenGL
{
    RenderAPI   header;
    RenderState render_state;

    u32 max_vertex_count;
    u32 vertex_count;
    Vertex* vertices;

    u32 max_index_count;
    u32 index_count;
    u16* indices;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GLuint program;

    opengl_function(glBindBuffer);
    opengl_function(glBindVertexArray);
    opengl_function(glBufferData);
    opengl_function(glGenBuffers);
    opengl_function(glGenVertexArrays);
    opengl_function(glVertexAttribPointer);
    opengl_function(glEnableVertexAttribArray);
    opengl_function(glCreateShader);
    opengl_function(glShaderSource);
    opengl_function(glCompileShader);
    opengl_function(glCreateProgram);
    opengl_function(glAttachShader);
    opengl_function(glLinkProgram);
    opengl_function(glValidateProgram);
    opengl_function(glGetProgramiv);
    opengl_function(glGetShaderInfoLog);
    opengl_function(glGetProgramInfoLog);
    opengl_function(glDeleteShader);
    opengl_function(glUseProgram);
};

RenderState* opengl_begin_frame(OpenGL* opengl, v2u draw_space);
void opengl_end_frame(OpenGL* opengl, RenderState* render_state);
void init_opengl(OpenGL* opengl);