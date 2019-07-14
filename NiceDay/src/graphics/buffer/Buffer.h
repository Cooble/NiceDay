#pragma once
#include <glad/glad.h>

struct VertexBufferElement
{
	unsigned int type;
	unsigned char normalized;
	unsigned int count;

	inline bool isIType() const
	{
		switch (type)
		{
		case GL_UNSIGNED_INT:
		case GL_UNSIGNED_SHORT:
		case GL_UNSIGNED_BYTE:
			return true;
		default:
			return false;
		}
	}

	static unsigned int getByteCount(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT: return 4;
		case GL_UNSIGNED_INT: return 4;
		case GL_UNSIGNED_SHORT: return 2;
		case GL_UNSIGNED_BYTE: return 1;
		}
		return 0;
	}
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
		ASSERT(false);
	}

	template <>
	void push<float>(unsigned int count)
	{
		m_elements.push_back({GL_FLOAT,GL_FALSE, count});
		m_stride += VertexBufferElement::getByteCount(GL_FLOAT) * count;
	}

	template <>
	void push<unsigned int>(unsigned int count)
	{
		m_elements.push_back({GL_UNSIGNED_INT,GL_FALSE, count});
		m_stride += VertexBufferElement::getByteCount(GL_UNSIGNED_INT) * count;
	}

	template <>
	void push<unsigned short>(unsigned int count)
	{
		m_elements.push_back({GL_UNSIGNED_SHORT,GL_FALSE, count});
		m_stride += VertexBufferElement::getByteCount(GL_UNSIGNED_SHORT) * count;
	}

	template <>
	void push<unsigned char>(unsigned int count)
	{
		m_elements.push_back({GL_UNSIGNED_BYTE,GL_FALSE, count});
		m_stride += VertexBufferElement::getByteCount(GL_UNSIGNED_BYTE) * count;
	}

	inline const std::vector<VertexBufferElement> getElements() const { return m_elements; }
	inline unsigned int getStride() const { return m_stride; }
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
