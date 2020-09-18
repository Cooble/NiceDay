#include "ndpch.h"
#include "VertexArray.h"
#include "graphics/Renderer.h"
#include "platform/OpenGL/GLVertexArray.h"

VertexArray* VertexArray::create()
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new GLVertexArray();
	default:
		ASSERT(false, "Invalid API");
		return nullptr;
	}
}


