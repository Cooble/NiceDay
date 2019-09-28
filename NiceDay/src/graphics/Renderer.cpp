#include "ndpch.h"
#include "Renderer.h"

#include <glad/glad.h>
#include "platform/OpenGL/GLRenderer.h"

GraphicsAPI Renderer::s_api = GraphicsAPI::OpenGL;



Renderer::Renderer() = default;


Renderer::~Renderer()= default;

void Renderer::draw(const VertexArray& vao, const Shader& shader, const IndexBuffer& ibo)
{
	vao.bind();
	shader.bind();
	ibo.bind();

	GLCall(glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT,nullptr));
}

void Renderer::clear()
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT));
}
