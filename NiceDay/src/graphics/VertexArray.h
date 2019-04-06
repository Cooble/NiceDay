#pragma once
#include "VertexBufferLayout.h"
#include "VertexBuffer.h"
class VertexArray
{
private:
	unsigned int m_id;
	
public:
	VertexArray();
	~VertexArray();
	void addBuffer(const VertexBuffer& vbo, const VertexBufferLayout& layout);

	void bind() const;
	void unbind() const;
};

