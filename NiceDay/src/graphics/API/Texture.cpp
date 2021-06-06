#include "ndpch.h"
#include "Texture.h"

#include "platform/OpenGL/GLTexture.h"
#include "graphics/Renderer.h"

namespace nd {

Texture* Texture::create(const TextureInfo& info)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new internal::GLTexture(info);

	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
}
