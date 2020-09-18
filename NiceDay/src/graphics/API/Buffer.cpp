﻿#include "ndpch.h"
#include "Buffer.h"
#include "graphics/Renderer.h"
#include "platform/OpenGL/GLBuffer.h"

VertexBufferLayout::VertexBufferLayout()
	:m_stride(0)
{
}

VertexBufferLayout::VertexBufferLayout(std::initializer_list<VertexBufferElement> list):m_stride(0)
{
	for (auto element : list)
		this->pushElement(element);
}

VertexBuffer* VertexBuffer::create(void* vertices, uint32_t size, BufferUsage usage)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new OpenGLVertexBuffer(vertices, size,usage);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}

IndexBuffer* IndexBuffer::create(uint32_t* vertices, uint32_t count,BufferUsage usage)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new OpenGLIndexBuffer(vertices, count,usage);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
