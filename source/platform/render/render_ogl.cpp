#include "render_ogl.h"

void init_opengl(OpenGL* opengl)
{
    GLuint vertex;
    opengl->glGenVertexArrays(1, &vertex);
    opengl->glBindVertexArray(vertex);

    opengl->glGenBuffers(1, &opengl->vbo);
    opengl->glGenBuffers(1, &opengl->ebo);
}