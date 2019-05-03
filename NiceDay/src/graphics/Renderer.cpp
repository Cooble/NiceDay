#include "ndpch.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void checkGLError(int line, const char* methodName, const char* file) {
	while (auto e = glGetError() != GL_NO_ERROR) {
		ND_ERROR("[OpenGL Error]: {}, {},	Line: {}, File: {} ", (GLenum)e, methodName, line, file);
	}
}

Renderer::Renderer()
{
}


Renderer::~Renderer()
{
}

void Renderer::draw(const VertexArray& vao, const Program& shader, const IndexBuffer& ibo)
{
	vao.bind();
	shader.bind();
	ibo.bind();

	Call(glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT,nullptr));
}

void Renderer::clear()
{
	Call(glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT));
}
