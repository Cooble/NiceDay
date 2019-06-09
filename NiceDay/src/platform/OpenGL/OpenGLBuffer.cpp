#include  "ndpch.h"
#include "OpenGLBuffer.h"
#include <glad/glad.h>
#include "graphics/Renderer.h"

/////////////////////////////////////////////////////////////////////////////////
//VertexBuffer //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size,uint32_t usage)
	:m_size(size),m_id(0)
{
	Call(glGenBuffers(1, &m_id));
	Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
	Call(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
	Call(glDeleteBuffers(1, &m_id));
}

void OpenGLVertexBuffer::bind() const
{
	Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}
void OpenGLVertexBuffer::unbind() const
{
	Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

uint32_t OpenGLVertexBuffer::getSize() const
{
	return m_size;
}

void OpenGLVertexBuffer::changeData(char* buff, uint32_t size, uint32_t offset)
{
	Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
	Call(glBufferSubData(GL_ARRAY_BUFFER, offset, size, buff));
}

/////////////////////////////////////////////////////////////////////////////////
//IndexBuffer////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, uint32_t count)
	:m_count(count)
{
	Call(glGenBuffers(1, &m_id));
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
	Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(uint32_t), data, GL_STATIC_DRAW));
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
	Call(glDeleteBuffers(1, &m_id));
}

void OpenGLIndexBuffer::bind() const
{
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}
void OpenGLIndexBuffer::unbind() const
{
	Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

uint32_t OpenGLIndexBuffer::getCount() const
{
	return m_count;
}

