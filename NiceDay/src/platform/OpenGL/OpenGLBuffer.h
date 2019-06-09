#pragma once
#include "graphics/buffer/Buffer.h"

class OpenGLVertexBuffer :public VertexBuffer
{
private:
	uint32_t m_id;
	uint32_t m_size;
public:
	OpenGLVertexBuffer(void* data, uint32_t size,uint32_t usage);
	~OpenGLVertexBuffer();
	void bind() const override;
	void unbind() const override;
	uint32_t getSize() const override;
	void changeData(char* buff, uint32_t size, uint32_t offset) override;
	
};


class OpenGLIndexBuffer :public IndexBuffer
{
private:
	uint32_t m_id;
	uint32_t m_count;
public:
	OpenGLIndexBuffer(uint32_t* data, uint32_t count);
	~OpenGLIndexBuffer();
	void bind() const override;
	void unbind() const override;
	uint32_t getCount() const override;

};
