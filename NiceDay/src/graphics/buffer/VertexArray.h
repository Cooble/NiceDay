#pragma once
#include "Buffer.h"
#include "glad/glad.h"
class VertexBufferLayout;

class VertexArray
{
	
public:
	virtual ~VertexArray() = default;
	virtual void addBuffer(const VertexBuffer& vbo, const VertexBufferLayout& layout)=0;
	static VertexArray* create();


	virtual void bind() const = 0;
	virtual void unbind() const=  0;


};
struct VertexBufferElement {
	unsigned int type;
	unsigned char normalized;
	unsigned int count;

	inline bool isIType() const {
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

	static unsigned int getByteCount(unsigned int type) {
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
	void push(unsigned int count) {
		static_assert(false);
	}
	template<>
	void push<float>(unsigned int count) {
		m_elements.push_back({ GL_FLOAT,GL_FALSE,count });
		m_stride += VertexBufferElement::getByteCount(GL_FLOAT)*count;
	}
	template<>
	void push<unsigned int>(unsigned int count) {
		m_elements.push_back({ GL_UNSIGNED_INT,GL_FALSE,count });
		m_stride += VertexBufferElement::getByteCount(GL_UNSIGNED_INT)*count;

	}
	template<>
	void push<unsigned short>(unsigned int count) {
		m_elements.push_back({ GL_UNSIGNED_SHORT,GL_FALSE,count });
		m_stride += VertexBufferElement::getByteCount(GL_UNSIGNED_SHORT)*count;
	}
	template<>
	void push<unsigned char>(unsigned int count) {
		m_elements.push_back({ GL_UNSIGNED_BYTE,GL_FALSE,count });
		m_stride += VertexBufferElement::getByteCount(GL_UNSIGNED_BYTE)*count;
	}

	inline const std::vector<VertexBufferElement> getElements() const { return m_elements; }
	inline unsigned int getStride() const { return m_stride; }


};


