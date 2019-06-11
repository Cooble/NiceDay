#include "ndpch.h"
#include "TestQuad.h"
#include "Renderer.h"

TestQuad::TestQuad(bool centered)
{
	float quad[] = {
		1, 0,
		1, 1,
		0, 0,
		0, 1
	};
	float quadCentered[] = {
		1, -1,
		1, 1,
		-1, -1,
		-1, 1
	};
	vbo = VertexBuffer::create(centered?quadCentered:quad,sizeof(float)*8,GL_STATIC_DRAW);
	vao = VertexArray::create();
	VertexBufferLayout l;
	l.push<float>(2);
	vao->addBuffer(*vbo, l);
	shader = new Shader("res/shaders/Test.shader");
}

TestQuad::~TestQuad()
{
	delete vao;
	delete vbo;
	delete shader;
}

void TestQuad::render(const glm::mat4& transform)
{
	shader->bind();
	shader->setUniformMat4("u_transform", transform);
	vao->bind();
	Call(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	shader->unbind();
	vao->unbind();
}

