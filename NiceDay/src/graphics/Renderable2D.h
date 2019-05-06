﻿#pragma once
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexBufferLayout.h"
#include "graphics/Program.h"
#include "glm/vec2.hpp"
class Renderable2D
{
private:
	VertexArray* m_vao;
	IndexBuffer* m_ibo;
	Program* m_program;
	glm::vec2* m_pos;
	glm::vec2* m_size;



public:

	
};