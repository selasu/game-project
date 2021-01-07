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

#define opengl_function(name) type_##name *name

struct OpenGL
{
    RenderAPI header;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    opengl_function(glBindBuffer);
    opengl_function(glBindVertexArray);
    opengl_function(glBufferData);
    opengl_function(glGenBuffers);
    opengl_function(glGenVertexArrays);
};

void init_opengl(OpenGL* opengl);