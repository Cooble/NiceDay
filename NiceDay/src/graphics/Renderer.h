#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "VertexArray.h"
#include "Program.h"
#include "IndexBuffer.h"
#ifndef LOADED_CALL_G

//this enables glGetError()

void checkGLError(int line, const char* methodName, const char* file);

#ifdef ND_DEBUG
#define Call(x) \
	while (glGetError() != GL_NO_ERROR);\
	x;\
	checkGLError(__LINE__,#x,__FILE__);
#else
#define Call(x) x;
#endif // DEBUG

#define LOADED_CALL_G
#endif // LOADED_CALL_G

	




class Renderer
{
public:
	Renderer();
	~Renderer();

	void draw(const VertexArray& vao, const Program& shader, const IndexBuffer& ibo);
	void clear();
};

