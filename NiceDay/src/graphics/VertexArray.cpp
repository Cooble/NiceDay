#include "ndpch.h"
#include "VertexArray.h"
#include "Renderer.h"

VertexArray::VertexArray()
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
		Call(glEnableVertexAttribArray(i));
		Call(glVertexAttribPointer(i, e.count, e.type, e.normalized, layout.getStride(), (const void*)offset));
		offset += e.count*VertexBufferElement::getByteCount(e.type);
	}


}

void VertexArray::bind() const
{
	Call(glBindVertexArray(m_id));
}

void VertexArray::unbind() const
{
	Call(glBindVertexArray(0));

}
