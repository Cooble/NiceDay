#include "ndpch.h"
#include "Shader.h"
#include <filesystem>
#include "graphics/Renderer.h"
#include "platform/OpenGL/GLShader.h"


Shader* Shader::create(const ShaderProgramSources& res)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new GLShader(res);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}

Shader* Shader::create(const std::string& filePath)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return new GLShader(filePath);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}



