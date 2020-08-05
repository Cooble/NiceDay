#pragma once
#include <glad/glad.h>
#include "graphics/GContext.h"

struct VertexBufferElement
{
	g_typ typ;
	bool normalized;
	uint32_t count;
	VertexBufferElement()=default;
	VertexBufferElement(g_typ typ,bool norm,uint32_t count):typ(typ),normalized(norm),count(count){}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_stride;
public:
	VertexBufferLayout();

	template <typename T>
	void push(unsigned int count)
	{
		ND_WARN("Shit ....");
		ASSERT(false,"fuk");
	}
	void pushElement(const VertexBufferElement& e)
	{
		m_elements.push_back(e);
		m_stride += GTypes::getSize(e.typ) * e.count;
	}

	template <>
	void push<float>(unsigned int count)
	{
		m_elements.emplace_back(g_typ::FLOAT, false, count);
		m_stride += GTypes::getSize(g_typ::FLOAT) * count;
	}

	template <>
	void push<unsigned int>(unsigned int count)
	{
		m_elements.emplace_back(g_typ::UNSIGNED_INT, false, count);
		m_stride += GTypes::getSize(g_typ::UNSIGNED_INT) * count;
	}

	template <>
	void push<unsigned short>(unsigned int count)
	{
		m_elements.emplace_back(g_typ::UNSIGNED_SHORT,false, count);
		m_stride += GTypes::getSize(g_typ::UNSIGNED_SHORT) * count;
	}

	template <>
	void push<unsigned char>(unsigned int count)
	{
		m_elements.emplace_back(g_typ::UNSIGNED_BYTE, false, count);
		m_stride += GTypes::getSize(g_typ::UNSIGNED_BYTE) * count;
	}

	const std::vector<VertexBufferElement>& getElements() const { return m_elements; }
	unsigned int getStride() const { return m_stride; }
};

enum class BufferUsage
{
	STATIC_DRAW = GL_STATIC_DRAW,
	DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
	STREAM_DRAW = GL_STREAM_DRAW,
};


class VertexBuffer
{
protected:
	VertexBufferLayout m_layout;
public:
	virtual ~VertexBuffer() = default;
	virtual void unbind() const = 0;
	virtual void bind() const = 0;
	virtual uint32_t getSize() const = 0;
	virtual void setLayout(const VertexBufferLayout& layout)  { this->m_layout = layout; }
	virtual const VertexBufferLayout& getLayout() const { return m_layout; }
	virtual void changeData(char* buff, uint32_t size, uint32_t offset) = 0;
	virtual void* mapPointer() = 0;
	virtual void unMapPointer() = 0;

	static VertexBuffer* create(void* vertices, uint32_t size, BufferUsage = BufferUsage::STATIC_DRAW);
};

class IndexBuffer
{
public:
	virtual ~IndexBuffer() = default;
	virtual void unbind() const = 0;
	virtual void bind() const = 0;
	virtual uint32_t getCount() const = 0;
	static IndexBuffer* create(uint32_t* vertices, uint32_t count,BufferUsage = BufferUsage::STATIC_DRAW);
};
