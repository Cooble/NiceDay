#include "ndpch.h"
#include "GLVertexArray.h"
#include "GLRenderer.h"
#include "graphics/GContext.h"
#include "GLContext.h"

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
	auto& ray = vbo.getLayout().getElements();

	for (unsigned int i = 0; i < ray.size(); i++) {
		auto& e = ray[i];
		
		GLCall(glEnableVertexAttribArray(m_atrib_point_index + i));
		if (GTypes::isIType(e.typ)) {
			GLCall(glVertexAttribIPointer(m_atrib_point_index + i, GTypes::getCount(e.typ), toGL(GTypes::getBase(e.typ)), vbo.getLayout().getStride(), (const void*)offset));
		}
		else {
			GLCall(glVertexAttribPointer(m_atrib_point_index + i, GTypes::getCount(e.typ), toGL(GTypes::getBase(e.typ)), e.normalized, vbo.getLayout().getStride(), (const void*)offset));
		}
		

		offset += GTypes::getSize(e.typ);
	}
	m_atrib_point_index += ray.size();
	unbind();
}

void GLVertexArray::addBuffer(const IndexBuffer& vio)
{
	bind();
	vio.bind();
	unbind();
}

void GLVertexArray::bind() const
{
	GLCall(glBindVertexArray(m_id));
}

void GLVertexArray::unbind() const
{
	GLCall(glBindVertexArray(0));

}
