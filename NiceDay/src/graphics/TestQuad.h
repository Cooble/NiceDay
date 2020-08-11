#pragma once
#include "API/Shader.h"
#include "API/Buffer.h"
#include "API/VertexArray.h"

struct TestQuad
{
	ShaderPtr shader;
	VertexBuffer* vbo;
	VertexArray* vao;
public:
	TestQuad(bool centered=false);
	~TestQuad();
	void render(const glm::mat4& transform);
	
};
