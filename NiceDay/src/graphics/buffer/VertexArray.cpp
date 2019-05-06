#include "ndpch.h"
#include "VertexArray.h"
#include "graphics/Renderer.h"

VertexArray::VertexArray()
:m_atrib_point_index(0)
{
	Call(glGenVertexArrays(1, &m_id));
}


VertexArray::~VertexArray()
{
	Call(glDeleteVertexArrays(1, &m_id));
}

void VertexArray::addBuffer(const VertexBuffer& vbo, const VertexBufferLayout& layout)
{
	bind();
	vbo.bind();
	unsigned int offset = 0;
	std::vector<VertexBufferElement> ray = layout.getElements();

	for (unsigned int i = 0; i < ray.size(); i++) {
		auto& e = ray[i];
		Call(glEnableVertexAttribArray(m_atrib_point_index+i));
		if (e.isIType()) {
			Call(glVertexAttribIPointer(m_atrib_point_index + i, e.count, e.type, layout.getStride(), (const void*)offset));
		}
		else {
			Call(glVertexAttribPointer(m_atrib_point_index + i, e.count, e.type, e.normalized, layout.getStride(), (const void*)offset));
		}

		offset += e.count*VertexBufferElement::getByteCount(e.type);
	}
	m_atrib_point_index+= ray.size();


}

void VertexArray::bind() const
{
	Call(glBindVertexArray(m_id));
}

void VertexArray::unbind() const
{
	Call(glBindVertexArray(0));

}