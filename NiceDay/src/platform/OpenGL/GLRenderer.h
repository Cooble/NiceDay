#pragma once
#include <glad/glad.h>

namespace nd::internal {

#ifdef ND_DEBUG

#define GLCall(x) { \
    for (int i = 0; i < 5 && glGetError() != GL_NO_ERROR; ++i); /* Clear previous errors */ \
    x; \
    nd::internal::checkGLError(__LINE__, #x, __FILE__); \
}

#define GLCall2(x) { \
    GLenum error; \
     ((error = glGetError()) != GL_NO_ERROR) { \
        std::cout << "OpenGL Error: " << error << std::endl; \
    } \
    x; \
    nd::internal::checkGLError(__LINE__, #x, __FILE__); \
}

#else
#define GLCall(x) x;
#endif // DEBUG

void checkGLError(int line, const char* method_name, const char* file);

}
