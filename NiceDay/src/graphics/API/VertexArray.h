#pragma once
#include "Buffer.h"

class VertexArray
{
	
public:
	virtual ~VertexArray() = default;
	virtual void addBuffer(const VertexBuffer& vbo)=0;
	virtual void addBuffer(const IndexBuffer& vio)=0;
	static VertexArray* create();


	virtual void bind() const = 0;
	virtual void unbind() const=  0;


};