#pragma once

#include <utility>
#include "ndpch.h"

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
	static Shader* create(const ShaderProgramSources& res);
	static Shader* create(const std::string& filePath);
	
	virtual ~Shader()=default;

	virtual void bind() const=0;
	virtual void unbind() const=0;


};
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

