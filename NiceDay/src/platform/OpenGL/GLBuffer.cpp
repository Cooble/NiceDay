#include  "ndpch.h"
#include "GLBuffer.h"
#include "GLRenderer.h"

/////////////////////////////////////////////////////////////////////////////////
//VertexBuffer //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size,BufferUsage usage)
	:m_size(size),m_id(0)
{
	GLCall(glGenBuffers(1, &m_id));
	bind();
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, (int)usage));
	unbind();
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_id));
}

void OpenGLVertexBuffer::bind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}
void OpenGLVertexBuffer::unbind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

uint32_t OpenGLVertexBuffer::getSize() const
{
	return m_size;
}

static bool OPENGL_MAPPING_BUFF_ACTIVE=false;
void* OpenGLVertexBuffer::mapPointer()
{
	ASSERT(!OPENGL_MAPPING_BUFF_ACTIVE, "Already mapping!");
	OPENGL_MAPPING_BUFF_ACTIVE = true;
	void* out;
	GLCall(out= glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	ASSERT(out, "[OpenGLVertexBuffer]: Cannot get map pointer to VBO, is it currently bound?");
	return out;
}

void OpenGLVertexBuffer::unMapPointer()
{
	GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
	OPENGL_MAPPING_BUFF_ACTIVE = false;
}

void OpenGLVertexBuffer::changeData(char* buff, uint32_t size, uint32_t offset)
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_id));
	GLCall(glBufferSubData(GL_ARRAY_BUFFER, offset, size, buff));
}

/////////////////////////////////////////////////////////////////////////////////
//IndexBuffer////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, uint32_t count,BufferUsage usage)
	:m_count(count)
{
	GLCall(glGenBuffers(1, &m_id));
	bind();
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(uint32_t), data, (int)usage));
	unbind();
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_id));
}

void OpenGLIndexBuffer::bind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}
void OpenGLIndexBuffer::unbind() const
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

uint32_t OpenGLIndexBuffer::getCount() const
{
	return m_count;
}

