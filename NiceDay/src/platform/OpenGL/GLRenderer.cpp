#include "ndpch.h"
#include "GLRenderer.h"

namespace nd::internal {

#define BREAK_ON_GL_ERROR 1

void checkGLError(int line, const char* method_name, const char* file)
{
	bool shit = false;
	while (auto e = glGetError() != GL_NO_ERROR)
	{
		shit = true;
		ND_ERROR("[OpenGL Error]: code:{}, {},	Line: {}, File: {} ", (GLenum)e, method_name, line, file);
	}
#if BREAK_ON_GL_ERROR
	if (shit)
		assert(false);
#endif
}
}
