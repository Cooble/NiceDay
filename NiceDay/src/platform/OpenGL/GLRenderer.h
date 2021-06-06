#pragma once
#include <glad/glad.h>

namespace nd::internal {

#ifdef ND_DEBUG
#define GLCall(x) {\
	while (glGetError() != GL_NO_ERROR);\
	x;\
	nd::internal::checkGLError(__LINE__,#x,__FILE__);}
#else
#define GLCall(x) x;
#endif // DEBUG

void checkGLError(int line, const char* method_name, const char* file);

}
