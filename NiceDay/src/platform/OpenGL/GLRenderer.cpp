#include "ndpch.h"
#include "GLRenderer.h"

namespace nd::internal
{
#define BREAK_ON_GL_ERROR 0

void checkGLError(int line, const char* method_name, const char* file)
{
	constexpr int MAX_TRIES = 1;

	bool shit = false;
	for (int i = 0, e; i < MAX_TRIES && (e = glGetError()) != GL_NO_ERROR; i++)
	{
		shit = true;
		//ND_ERROR("[OpenGL Error]: code:{}, {},	Line: {}, File: {} ", (GLenum)e, method_name, line, file); cannot use, it might get called after logger is destructed
		std::cout << "[OpenGL Error]: code:" << e << ", " << method_name << ", " << line << ", " << file << std::endl;
	}
#if BREAK_ON_GL_ERROR
	if (shit)
		assert(false);
#endif
}
}
