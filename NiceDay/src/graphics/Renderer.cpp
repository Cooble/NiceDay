#include "ndpch.h"
#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void checkGLError(int line, const char* method_name, const char* file) {
	while (auto e = glGetError() != GL_NO_ERROR) {
		ND_ERROR("[OpenGL Error]: {}, {},	Line: {}, File: {} ", (GLenum)e, method_name, line, file);
	}
}

Renderer::Renderer() = default;


Renderer::~Renderer()= default;

void Renderer::draw(const VertexArray& vao, const Program& shader, const IndexBuffer& ibo)
{
	vao.bind();
	shader.bind();
	ibo.bind();

	Call(glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT,nullptr));
}

void Renderer::draw(glm::mat4 trans, Sprite2D& sprite)
{
	sprite.getVAO().bind();
	sprite.getTexture().bind(0);
	sprite.getProgram().bind();
	sprite.getProgram().setUniformMat4("u_transform", trans*sprite.getModelMatrix());
	Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void Renderer::clear()
{
	Call(glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT));
}
