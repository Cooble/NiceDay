#include "ndpch.h"
#include "Renderer.h"

#include <glad/glad.h>
#include "platform/OpenGL/OpenGLRenderer.h"

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

void Renderer::draw(glm::mat4 trans, Sprite2D& sprite)
{
	sprite.getVAO().bind();
	sprite.getTexture().bind(0);
	sprite.getProgram().bind();
	sprite.getProgram().setUniformMat4("u_transform", trans*sprite.getModelMatrix());
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void Renderer::clear()
{
	GLCall(glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT));
}
