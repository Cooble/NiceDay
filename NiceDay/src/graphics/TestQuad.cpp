#include "ndpch.h"
#include "TestQuad.h"
#include "platform/OpenGL/GLRenderer.h"
#include "platform/OpenGL/GLShader.h"

namespace nd {

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
	vbo = VertexBuffer::create(centered ? quadCentered : quad, sizeof(float) * 8, BufferUsage::STATIC_DRAW);
	vao = VertexArray::create();
	VertexBufferLayout l{g_typ::VEC2};
	vbo->setLayout(l);
	vao->addBuffer(*vbo);
	shader = ShaderLib::loadOrGetShader("res/shaders/Test.shader");
}

TestQuad::~TestQuad()
{
	delete vao;
	delete vbo;
}

void TestQuad::render(const glm::mat4& transform)
{
	shader->bind();
	std::static_pointer_cast<internal::GLShader>(shader)->setUniformMat4("u_transform", transform);
	vao->bind();
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
	shader->unbind();
	vao->unbind();
}
}
