#include "ndpch.h"
#include "VertexArray.h"
#include "graphics/Renderer.h"
#include "platform/OpenGL/GLVertexArray.h"

namespace nd {

VertexArray* VertexArray::create()
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new internal::GLVertexArray();
	default:
		ASSERT(false, "Invalid API");
		return nullptr;
	}
}
}
