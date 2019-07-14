#include "ndpch.h"
#include "Texture.h"
#include "Renderer.h"
#include <stb_image.h>
#include "platform/OpenGL/OpenGLTexture.h"

Texture* Texture::create(TextureInfo& info)
{
	switch(Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new OpenGLTexture(info);

	default: 
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
