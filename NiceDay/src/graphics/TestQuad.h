#pragma once
#include "Shader.h"
#include "buffer/Buffer.h"
#include "buffer/VertexArray.h"

struct TestQuad
{
	Shader* shader;
	VertexBuffer* vbo;
	VertexArray* vao;
public:
	TestQuad(bool centered=false);
	~TestQuad();
	void render(const glm::mat4& transform);
	
};
