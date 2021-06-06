#include "ndpch.h"
#include "FrameBuffer.h"
#include "platform/OpenGL/GLFrameBuffer.h"
#include "graphics/Renderer.h"

namespace nd {
FrameBuffer* FrameBuffer::create(const FrameBufferInfo& info)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new internal::GLFrameBuffer(info);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
}
