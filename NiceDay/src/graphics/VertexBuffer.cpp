#include "ndpch.h"
#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int length,bool dynamic) {
	Call(glGenBuffers(1, &m_id));
	Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
	Call(glBufferData(GL_ARRAY_BUFFER, length, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
}
void VertexBuffer::changeData(const void* data, int size, int offset)
{
	Call(glBufferSubData(m_id,offset,size,data));
}

VertexBuffer::~VertexBuffer()
{
	Call(glDeleteBuffers(1, &m_id));
}

void VertexBuffer::bind() const
{
	Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}

void VertexBuffer::unbind() const
{
	Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
