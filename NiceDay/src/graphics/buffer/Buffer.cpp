#include "ndpch.h"
#include "Buffer.h"
#include "graphics/Renderer.h"
#include "platform/OpenGL/OpenGLBuffer.h"

VertexBuffer* VertexBuffer::create(void* vertices, uint32_t size,uint32_t usage)
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

IndexBuffer* IndexBuffer::create(uint32_t* vertices, uint32_t size)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new OpenGLIndexBuffer(vertices, size);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
