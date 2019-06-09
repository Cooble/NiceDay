#include "ndpch.h"
#include "FrameBuffer.h"
#include "platform/OpenGL/OpenGLFrameBuffer.h"
#include "graphics/Renderer.h"


FrameBuffer* FrameBuffer::create()
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new OpenGLFrameBuffer();
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
