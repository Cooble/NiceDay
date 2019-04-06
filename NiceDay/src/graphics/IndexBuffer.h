#pragma once
#include<glad/glad.h>

class IndexBuffer
{
public:
	IndexBuffer(const unsigned int* data, unsigned int length);
	~IndexBuffer();

	void bind() const;
	void unbind() const;

	inline unsigned int getCount() const { return m_count; }
private:
	unsigned int m_id;
	unsigned int m_count;
};

