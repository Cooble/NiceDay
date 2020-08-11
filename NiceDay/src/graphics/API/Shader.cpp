#include "ndpch.h"
#include "Shader.h"
#include <filesystem>
#include "graphics/Renderer.h"
#include "platform/OpenGL/GLShader.h"

static void extractLayoutIn(ShaderLayout& out, const std::string& vertexSrc)
{
	std::string noComments = vertexSrc;
	SUtil::removeComments(noComments);
	defaultable_vector<UniformElement> list;
	for (auto it = SUtil::SplitIterator<true, const char*, true>(noComments, ";"); it; ++it)
	{
		auto line = *it;
		for (auto it = SUtil::SplitIterator<true, const char*, true>(line, "\t\n ()="); it; ++it)
		{
			auto word = *it;
			if (word == "layout") {
				UniformElement e;

				ASSERT(*++it == "location", "Error while extracting shader layout");//location
				int ind=  std::stoi(std::string(*++it));//index
				ASSERT(*++it=="in","Error while extracting shader layout");//in
				e.type=GTypes::getType(*++it);//type
				e.name = *++it;//name
				e.offset = 0;
				e.arraySize = 1;
				list[ind] = std::move(e);
			}
		}
	}
	UniformLayout layout;
	layout.name = "VERTEX";
	layout.size = 0;
	layout.prefixName = "";
	layout.stages = 0;
	for (UniformElement& uniform_element : list)
		layout.elements.push_back(std::move(uniform_element));
	out.structs.push_back(std::move(layout));
}
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
								if (!SUtil::startsWith(e.name, s.prefixName)) {//otherwise we would add prefix in each stage
									e.name = s.prefixName + "." + e.name;
								}
								else break;
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
		extractLayoutIn(out, res.vertexSrc);
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

ShaderPtr Shader::create(const ShaderProgramSources& res)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return MakeRef<GLShader>(res);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}

ShaderPtr Shader::create(const std::string& filePath)
{
	switch (Renderer::getAPI())
	{
	case GraphicsAPI::OpenGL:
		return MakeRef<GLShader>(filePath);
	default:
		ASSERT(false, "Invalid RenderAPI");
		return nullptr;
	}
}
