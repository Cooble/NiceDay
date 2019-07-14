#include "ndpch.h"
#include "OpenGLRenderer.h"


void checkGLError(int line, const char* method_name, const char* file) {
	while (auto e = glGetError() != GL_NO_ERROR) {
		ND_ERROR("[OpenGL Error]: {}, {},	Line: {}, File: {} ", (GLenum)e, method_name, line, file);
	}
}
