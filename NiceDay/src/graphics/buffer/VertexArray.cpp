#include "ndpch.h"
#include "VertexArray.h"
#include "graphics/Renderer.h"
#include "platform/OpenGL/OpenGLVertexArray.h"

VertexArray* VertexArray::create()
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new OpenGLVertexArray();
	default:
		ASSERT(false, "Invalid API");
		return nullptr;
	}
}

VertexBufferLayout::VertexBufferLayout()
	:m_stride(0)
{
}
