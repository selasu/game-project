#include "ogl.h"

#define DECLARE(glFunction) type_##glFunction func_##glFunction = nullptr
#define LOAD(glFunction) glFunction = (type_##glFunction)load_function(#glFunction)

DECLARE(glGetString);
DECLARE(glClearColor);
DECLARE(glClear);
DECLARE(glGenBuffers);
DECLARE(glBindBuffer);
DECLARE(glBufferData);
DECLARE(glCreateShader);
DECLARE(glShaderSource);
DECLARE(glCompileShader);
DECLARE(glGetShaderiv);
DECLARE(glGetShaderInfoLog);
DECLARE(glCreateProgram);
DECLARE(glAttachShader);
DECLARE(glLinkProgram);
DECLARE(glGetProgramiv);
DECLARE(glGetProgramInfoLog);
DECLARE(glUseProgram);
DECLARE(glDeleteShader);
DECLARE(glVertexAttribPointer);
DECLARE(glEnableVertexAttribArray);
DECLARE(glGenVertexArrays);
DECLARE(glBindVertexArray);
DECLARE(glDrawArrays);
DECLARE(glDrawElements);
DECLARE(glViewport);
DECLARE(glDeleteVertexArrays);
DECLARE(glDeleteBuffers);
DECLARE(glDeleteProgram);
DECLARE(glUniform1f);
DECLARE(glUniform1i);
DECLARE(glGetUniformLocation);

int load_opengl_functions(std::function<void*(const char*)> load_function)
{
	LOAD(glGetString);
	if (!glGetString || !glGetString(GL_VERSION)) return 0;

	LOAD(glClearColor);
	LOAD(glClear);
	LOAD(glGenBuffers);
	LOAD(glBindBuffer);
	LOAD(glBufferData);
	LOAD(glCreateShader);
	LOAD(glShaderSource);
	LOAD(glCompileShader);
	LOAD(glGetShaderiv);
	LOAD(glGetShaderInfoLog);
	LOAD(glCreateProgram);
	LOAD(glAttachShader);
	LOAD(glLinkProgram);
	LOAD(glGetProgramiv);
	LOAD(glGetProgramInfoLog);
	LOAD(glUseProgram);
	LOAD(glDeleteShader);
	LOAD(glVertexAttribPointer);
	LOAD(glEnableVertexAttribArray);
	LOAD(glGenVertexArrays);
	LOAD(glBindVertexArray);
	LOAD(glDrawArrays);
	LOAD(glDrawElements);
	LOAD(glViewport);
	LOAD(glDeleteVertexArrays);
	LOAD(glDeleteBuffers);
	LOAD(glDeleteProgram);
	LOAD(glUniform1f);
	LOAD(glUniform1i);
	LOAD(glGetUniformLocation);

	return 1;
}