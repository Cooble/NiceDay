#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int length,bool dynamic=false);
	void changeData(const void* data, int size, int offset);
	~VertexBuffer();

	void bind()const;
	void unbind()const;
private:
	unsigned int m_id;
};

