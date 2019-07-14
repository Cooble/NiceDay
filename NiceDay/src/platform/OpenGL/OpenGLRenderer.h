#pragma once
#include <glad/glad.h>
#ifdef ND_DEBUG
#define GLCall(x) \
	while (glGetError() != GL_NO_ERROR);\
	x;\
	checkGLError(__LINE__,#x,__FILE__);
#else
#define GLCall(x) x;
#endif // DEBUG

void checkGLError(int line, const char* method_name, const char* file);


class GLRenderer
{
public:
	
};
