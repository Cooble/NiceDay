#pragma once
#include "graphics/buffer/VertexArray.h"

class OpenGLVertexArray : public VertexArray
{
private:
	unsigned int m_id;
	unsigned int m_atrib_point_index;

public:
	OpenGLVertexArray();
	~OpenGLVertexArray();
	void addBuffer(const VertexBuffer& vbo) override;

	void bind() const override;
	void unbind() const override;
};
