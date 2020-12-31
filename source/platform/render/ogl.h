#pragma once

#include <functional>

#if defined(_WIN32) && !defined(APIENTRY)
	#define APIENTRY __stdcall
#endif

#ifndef APIENTRYP
	#define APIENTRYP APIENTRY*
#endif

int load_opengl_functions(std::function<void*(const char*)> loader);

struct OGLVersion
{
    int major;
    int minor;
};

static OGLVersion ogl_version;

extern "C"
{
    typedef unsigned int    GLenum;
	typedef unsigned char   GLboolean;
	typedef unsigned int    GLbitfield;
	typedef signed char     GLbyte;
	typedef short           GLshort;
	typedef int             GLint;
	typedef int             GLsizei;
	typedef unsigned char   GLubyte;
	typedef unsigned short  GLushort;
	typedef unsigned int    GLuint;
	typedef float           GLfloat;
	typedef float           GLclampf;
	typedef double          GLdouble;
	typedef double          GLclampd;
	typedef void            GLvoid;
	typedef signed long int GLsizeiptr;
	typedef char            GLchar;

    #define GL_VERSION              0x1F02
	#define GL_TRUE	                1
	#define GL_FALSE                0
	#define GL_COLOR_BUFFER_BIT     0x00004000
	#define GL_ARRAY_BUFFER         0x8892
	#define GL_DEPTH_BUFFER_BIT     0x0100
	#define GL_ELEMENT_ARRAY_BUFFER 0x8893
	#define GL_STATIC_DRAW          0x88E4
	#define GL_COMPILE_STATUS       0x8B81
	#define GL_LINK_STATUS          0x8B82
	#define GL_FRAGMENT_SHADER      0x8B30
	#define GL_VERTEX_SHADER        0x8B31
	#define GL_TRIANGLES            0x0004
	#define GL_UNSIGNED_INT         0x1405
	#define GL_FLOAT                0x1406

    typedef const GLubyte*(APIENTRYP type_glGetString)(GLenum name);
	extern type_glGetString func_glGetString;
	#define glGetString func_glGetString

	typedef GLvoid(APIENTRYP type_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	extern type_glClearColor func_glClearColor;
	#define glClearColor func_glClearColor

	typedef GLvoid(APIENTRYP type_glClear)(GLbitfield mask);
	extern type_glClear func_glClear;
	#define glClear func_glClear

	typedef GLvoid(APIENTRYP type_glGenBuffers)(GLsizei n, GLuint* buffers);
	extern type_glGenBuffers func_glGenBuffers;
	#define glGenBuffers func_glGenBuffers

	typedef GLvoid(APIENTRYP type_glBindBuffer)(GLenum target, GLuint buffer);
	extern type_glBindBuffer func_glBindBuffer;
	#define	glBindBuffer func_glBindBuffer

	typedef GLvoid(APIENTRYP type_glBufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	extern type_glBufferData func_glBufferData;
	#define	glBufferData func_glBufferData

	typedef GLuint(APIENTRYP type_glCreateShader)(GLenum shader_type);
	extern type_glCreateShader func_glCreateShader;
	#define glCreateShader func_glCreateShader

	typedef GLvoid(APIENTRYP type_glShaderSource)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
	extern type_glShaderSource func_glShaderSource;
	#define glShaderSource func_glShaderSource

	typedef GLvoid(APIENTRYP type_glCompileShader)(GLuint shader);
	extern type_glCompileShader func_glCompileShader;
	#define glCompileShader func_glCompileShader

	typedef GLvoid(APIENTRYP type_glGetShaderiv)(GLuint shader, GLenum pname, GLuint* params);
	extern type_glGetShaderiv func_glGetShaderiv;
	#define glGetShaderiv func_glGetShaderiv

	typedef GLvoid(APIENTRYP type_glGetShaderInfoLog)(GLuint shader, GLsizei max_length, GLsizei* length, GLchar* info_log);
	extern type_glGetShaderInfoLog func_glGetShaderInfoLog;
	#define glGetShaderInfoLog func_glGetShaderInfoLog

	typedef GLuint(APIENTRYP type_glCreateProgram)(GLvoid);
	extern type_glCreateProgram func_glCreateProgram;
	#define glCreateProgram func_glCreateProgram

	typedef GLvoid(APIENTRYP type_glAttachShader)(GLuint program, GLuint shader);
	extern type_glAttachShader func_glAttachShader;
	#define glAttachShader func_glAttachShader

	typedef GLvoid(APIENTRYP type_glLinkProgram)(GLuint program);
	extern type_glLinkProgram func_glLinkProgram;
	#define glLinkProgram func_glLinkProgram

	typedef GLvoid(APIENTRYP type_glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
	extern type_glGetProgramiv func_glGetProgramiv;
	#define glGetProgramiv func_glGetProgramiv

	typedef GLvoid(APIENTRYP type_glGetProgramInfoLog)(GLuint program, GLsizei max_length, GLsizei* length, GLchar* info_log);
	extern type_glGetProgramInfoLog func_glGetProgramInfoLog;
	#define glGetProgramInfoLog func_glGetProgramInfoLog

	typedef GLvoid(APIENTRYP type_glUseProgram)(GLuint program);
	extern type_glUseProgram func_glUseProgram;
	#define glUseProgram func_glUseProgram

	typedef GLvoid(APIENTRYP type_glDeleteShader)(GLuint shader);
	extern type_glDeleteShader func_glDeleteShader;
	#define glDeleteShader func_glDeleteShader

	typedef GLvoid(APIENTRYP type_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
	extern type_glVertexAttribPointer func_glVertexAttribPointer;
	#define glVertexAttribPointer func_glVertexAttribPointer
	
	typedef GLvoid(APIENTRYP type_glEnableVertexAttribArray)(GLuint index);
	extern type_glEnableVertexAttribArray func_glEnableVertexAttribArray;
	#define glEnableVertexAttribArray func_glEnableVertexAttribArray

	typedef GLvoid(APIENTRYP type_glGenVertexArrays)(GLsizei n, GLuint* arrays);
	extern type_glGenVertexArrays func_glGenVertexArrays;
	#define glGenVertexArrays func_glGenVertexArrays
	
	typedef GLvoid(APIENTRYP type_glBindVertexArray)(GLuint array);
	extern type_glBindVertexArray func_glBindVertexArray;
	#define glBindVertexArray func_glBindVertexArray

	typedef GLvoid(APIENTRYP type_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
	extern type_glDrawArrays func_glDrawArrays;
	#define glDrawArrays func_glDrawArrays

	typedef GLvoid(APIENTRYP type_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);
	extern type_glDrawElements func_glDrawElements;
	#define glDrawElements func_glDrawElements

	typedef GLvoid(APIENTRYP type_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
	extern type_glViewport func_glViewport;
	#define glViewport func_glViewport

	typedef GLvoid(APIENTRYP type_glDeleteVertexArrays)(GLsizei n, const GLuint* arrays);
	extern type_glDeleteVertexArrays func_glDeleteVertexArrays;
	#define glDeleteVertexArrays func_glDeleteVertexArrays

	typedef GLvoid(APIENTRYP type_glDeleteBuffers)(GLsizei n, const GLuint* buffers);
	extern type_glDeleteBuffers func_glDeleteBuffers;
	#define glDeleteBuffers func_glDeleteBuffers

	typedef GLvoid(APIENTRYP type_glDeleteProgram)(GLuint program);
	extern type_glDeleteProgram func_glDeleteProgram;
	#define glDeleteProgram func_glDeleteProgram

	typedef GLvoid(APIENTRYP type_glUniform1f)(GLint location, GLfloat v0);
	extern type_glUniform1f func_glUniform1f;
	#define glUniform1f func_glUniform1f

	typedef GLvoid(APIENTRYP type_glUniform1i)(GLint location, GLint v0);
	extern type_glUniform1i func_glUniform1i;
	#define glUniform1i func_glUniform1i

	typedef GLint(APIENTRYP type_glGetUniformLocation)(GLuint program, const GLchar* name);
	extern type_glGetUniformLocation func_glGetUniformLocation;
	#define glGetUniformLocation func_glGetUniformLocation
}