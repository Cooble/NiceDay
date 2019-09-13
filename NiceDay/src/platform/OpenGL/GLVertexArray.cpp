#include "ndpch.h"
#include "GLVertexArray.h"
#include "GLRenderer.h"

GLVertexArray::GLVertexArray()
	:m_atrib_point_index(0)
{
	GLCall(glGenVertexArrays(1, &m_id));
}

GLVertexArray::~GLVertexArray()
{
	GLCall(glDeleteVertexArrays(1,&m_id));
}

void GLVertexArray::addBuffer(const VertexBuffer& vbo)
{
	bind();
	vbo.bind();
	unsigned int offset = 0;
	std::vector<VertexBufferElement> ray = vbo.getLayout().getElements();

	for (unsigned int i = 0; i < ray.size(); i++) {
		auto& e = ray[i];
		GLCall(glEnableVertexAttribArray(m_atrib_point_index + i));
		if (e.isIType()) {
			GLCall(glVertexAttribIPointer(m_atrib_point_index + i, e.count, e.type, vbo.getLayout().getStride(), (const void*)offset));
		}
		else {
			GLCall(glVertexAttribPointer(m_atrib_point_index + i, e.count, e.type, e.normalized, vbo.getLayout().getStride(), (const void*)offset));
		}

		offset += e.count*VertexBufferElement::getByteCount(e.type);
	}
	m_atrib_point_index += ray.size();
}

void GLVertexArray::bind() const
{
	GLCall(glBindVertexArray(m_id));
}

void GLVertexArray::unbind() const
{
	GLCall(glBindVertexArray(0));

}
