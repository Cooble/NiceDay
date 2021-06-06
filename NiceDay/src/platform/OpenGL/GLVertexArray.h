#pragma once
#include "graphics/API/VertexArray.h"

namespace nd::internal {

class GLVertexArray : public VertexArray
{
private:
	unsigned int m_id;
	unsigned int m_atrib_point_index;

public:
	GLVertexArray();
	~GLVertexArray() override;
	void addBuffer(const VertexBuffer& vbo) override;
	void addBuffer(const IndexBuffer& vio) override;


	void bind() const override;
	void unbind() const override;
};
}
