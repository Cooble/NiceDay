#include "ndpch.h"
#include "FontObject.h"

namespace nd {

FontObject::FontObject(int size)
	: m_textMesh(size)
{
	m_vbo = VertexBuffer::create(m_textMesh.getSrc(), m_textMesh.getMaxByteSize());
	VertexBufferLayout layout{
		g_typ::VEC2, //pos
		g_typ::VEC2, //uv
	};
	m_vbo->setLayout(layout);
	m_vao = VertexArray::create();
	m_vao->addBuffer(*m_vbo);
}

void FontObject::updateMesh()
{
	m_vbo->changeData((char*)m_textMesh.getSrc(), m_textMesh.getMaxByteSize(), 0);
}

void FontObject::bind()
{
	m_vao->bind();
}

void FontObject::unbind()
{
	m_vao->unbind();
}
}
