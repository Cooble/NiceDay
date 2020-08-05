#include "ndpch.h"
#include "Shader.h"
#include <filesystem>
#include "graphics/Renderer.h"
#include "platform/OpenGL/GLShader.h"


static void extractLayoutSingle(ShaderLayout& out, RenderStage stage, const std::string& src, bool expandPath)
{
	std::string noComments=src;
	SUtil::removeComments(noComments);
	UniformLayout currentStruct;
	UniformElement element;
	bool openStruct = false;
	int offset = 0;

	for (auto it = SUtil::SplitIterator<true,const char*, true>(noComments, "; \t\n"); it; ++it)
	{
		auto word = *it;
		if (!openStruct)
		{
			if (word == "uniform")
			{
				auto structName = *(++it);
				auto structPrefixName = *(++it);
				for (auto& s : out.structs)
					if (s.name == structName)
					{
						s.prefixName = structPrefixName;
						if(expandPath)
							for (auto& e : s.elements)
								e.name = s.prefixName + "." + e.name;
						break;
					}
			}
			else if (word == "struct")
			{
				currentStruct.name = *(++it);
				for (auto& s : out.structs)
					if (s.name == currentStruct.name)
					{
						s.stages |= (int)stage;
						currentStruct.name = "";
						break;
					}
				
			}
			else if (word == "{" && !currentStruct.name.empty())
				openStruct = true;
		}
		else
		{
			if (word == "}")
			{
				openStruct = false;
				currentStruct.stages |= (int)stage;
				currentStruct.size = offset;
				out.structs.push_back(std::move(currentStruct));
				currentStruct = UniformLayout();
				offset = 0;
			}
			else
			{
				element.type = GTypes::getType(word);
				if (element.type == g_typ::INVALID)
					throw std::string("Invalid type");
				auto nameIt = SUtil::SplitIterator<true,const char*,true>(*(++it), " []");
				element.name = *nameIt;
				if (++nameIt)
				{
					element.arraySize = std::stoi(std::string(*nameIt));
				}
				element.offset = offset;
				offset += GTypes::getSize(element.type) * element.arraySize;
				currentStruct.elements.push_back(std::move(element));
				element = UniformElement();
			}
		}
	}
}

ShaderLayout Shader::extractLayout(const ShaderProgramSources& res,bool expandPath)
{
	ShaderLayout out;
	try
	{
		extractLayoutSingle(out, RenderStage::VERTEX, res.vertexSrc, expandPath);
		extractLayoutSingle(out, RenderStage::FRAGMENT, res.fragmentSrc, expandPath);
		if (res.geometrySrc.size())
			extractLayoutSingle(out, RenderStage::GEOMETRY, res.geometrySrc, expandPath);
		return out;
	}
	catch (...)
	{
		ND_ERROR("Error while parsing shader vertex:\n\n{}\n\nfragment:\n\n{}\n\ngeometry:\n\n{}",res.vertexSrc,res.fragmentSrc,res.geometrySrc);
		return out;
	}
}

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
