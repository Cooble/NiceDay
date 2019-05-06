#include "ndpch.h"
#include "IndexBuffer.h"
#include "graphics/Renderer.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
	:m_count(count)
{
	Call(glGenBuffers(1, &m_id));
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
	Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}


IndexBuffer::~IndexBuffer()
{
	Call(glDeleteBuffers(1, &m_id));
}

void IndexBuffer::bind() const
{
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}

void IndexBuffer::unbind() const
{
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
