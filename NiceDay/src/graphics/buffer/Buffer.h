#pragma once

class VertexBuffer
{
public:
	virtual ~VertexBuffer() = default;
	virtual void unbind() const = 0;
	virtual void bind() const = 0;
	virtual uint32_t getSize() const = 0;
	virtual void changeData(char* buff, uint32_t size, uint32_t offset)=0;

	static VertexBuffer* create(void* vertices, uint32_t size,uint32_t usage);
};

class IndexBuffer
{
public:
	virtual ~IndexBuffer() = default;
	virtual void unbind() const = 0;
	virtual void bind() const = 0;
	virtual uint32_t getCount() const = 0;
	static IndexBuffer* create(uint32_t* vertices, uint32_t count);

};
