#pragma once
#include "graphics/API/VertexArray.h"

class GLVertexArray : public VertexArray
{
private:
	unsigned int m_id;
	unsigned int m_atrib_point_index;

public:
	GLVertexArray();
	~GLVertexArray();
	void addBuffer(const VertexBuffer& vbo) override;

	void bind() const override;
	void unbind() const override;
};
