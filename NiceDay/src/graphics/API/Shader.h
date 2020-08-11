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

	UniformElement* getElementByName(const std::string& name)
	{
		for (auto& element : elements)
			if (element.name == name)
				return &element;
		return nullptr;
	}
	const UniformElement* getElementByName(const std::string& name) const
	{
		for (auto& element : elements)
			if (element.name == name)
				return &element;
		return nullptr;
	}

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

inline bool operator==(UniformElement* const& lhs, const UniformElement& rhs)
{
	return lhs->type == rhs.type && lhs->name == rhs.name && lhs->arraySize == rhs.arraySize;
}

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
	typedef Ref<Shader> ShaderPtr;
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
	static ShaderPtr create(const ShaderProgramSources& res);
	static ShaderPtr create(const std::string& filePath);
	
	virtual ~Shader()=default;

	virtual void bind() const=0;
	virtual void unbind() const=0;
	virtual const ShaderLayout& getLayout() const = 0;

	virtual const std::string& getFilePath() const = 0;
};

typedef Ref<Shader> ShaderPtr;


class ShaderLib
{
private:
	inline static std::unordered_map<std::string, ShaderPtr> m_shaders;
public:
	// if neccessary, loads shader, otherwise retrives already loaded
	static ShaderPtr loadOrGetShader(const std::string& path)
	{
		auto it = m_shaders.find(path);
		if (it == m_shaders.end()) {
			auto t = Shader::create(path);
			m_shaders[path] = t;
			return t;
		}
		return it->second;
	}
	static void unloadAll()
	{
		m_shaders.clear();
	}
};

