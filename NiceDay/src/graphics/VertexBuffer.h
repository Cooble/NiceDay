#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int length);
	~VertexBuffer();

	void bind()const;
	void unbind()const;
private:
	unsigned int m_id;
};

