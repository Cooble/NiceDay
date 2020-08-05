#pragma once

#include <utility>
//#include "ndpch.h"
#include "graphics/GContext.h"

struct UniformElement
{
	g_typ type = g_typ::INVALID;
	int arraySize = 1;
	int offset = 0;
	std::string name;
	UniformElement(g_typ typ,int arraySize,int offset, std::string name):type(typ),arraySize(arraySize),offset(offset),name(
		                                                                     std::move(name)){}
	UniformElement() = default;
};

struct UniformLayout
{
	std::string name;
	std::string prefixName;
	std::vector<UniformElement> elements;

	//stores bits of RenderStage
	uint32_t stages=0;

	//in bytes, of uniform buffer
	size_t size;
public:
	void emplaceElement(g_typ type,int arraySize,const std::string& name)
	{
		elements.emplace_back(type, arraySize, (int)size, name);
		size += GTypes::getSize(type);
	}
};
struct ShaderLayout
{
	std::vector<UniformLayout> structs;
	const UniformLayout* getLayoutByName(const char* name) const
	{
		for(auto& s:structs)
			if (s.name == name)
				return &s;
		return nullptr;
	}
};


class Shader
{
public:
	struct ShaderProgramSources
	{
		std::string vertexSrc;
		std::string fragmentSrc;
		std::string geometrySrc;
		ShaderProgramSources(std::string vertex,std::string fragment,std::string geometry = "")
			:vertexSrc(std::move(vertex)), fragmentSrc(std::move(fragment)),geometrySrc(std::move(geometry)) {}

	};
	/**
	 * Extract uniform structs layout
	 * if expand path
	 *	name of elements will contain prefix e.g. "myUBO.lightPos" instead of just "lightPos"
	 */
	static ShaderLayout extractLayout(const ShaderProgramSources& res,bool expandPath=true);
	static Shader* create(const ShaderProgramSources& res);
	static Shader* create(const std::string& filePath);
	
	virtual ~Shader()=default;

	virtual void bind() const=0;
	virtual void unbind() const=0;
	virtual const ShaderLayout& getLayout() const = 0;


};

typedef Ref<Shader> ShaderPtr;


class ShaderLib
{
private:
	inline static std::unordered_map<std::string, Shader*> m_shaders;
public:
	// if neccessary, loads shader, otherwise retrives already loaded
	inline static Shader* loadOrGetShader(const std::string& path)
	{
		if(m_shaders.find(path)==m_shaders.end())
		{
			m_shaders[path] = Shader::create(path);
		}
		return m_shaders[path];
	}
	inline static void unloadAll()
	{
		for(auto& s:m_shaders)
			delete s.second;
		m_shaders.clear();
	}
};

