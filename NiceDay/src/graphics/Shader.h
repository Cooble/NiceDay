#pragma once

#include "ndpch.h"


#define BREAK_IF_SHADER_COMPILE_ERROR 1

static const char* s_current_file="null";
class Shader
{
private:
	unsigned int m_id;
	int getUniformLocation(const std::string& name);
	std::unordered_map<std::string, int> cache;
public:
	struct ShaderProgramSources
	{
		std::string vertexSrc;
		std::string fragmentSrc;
		std::string geometrySrc;
		ShaderProgramSources(const std::string& vertex,const std::string& fragment,const std::string& geometry = "")
			:vertexSrc(vertex), fragmentSrc(fragment),geometrySrc(geometry) {}

	};
	Shader(const ShaderProgramSources& src);
	Shader(const std::string &file_path);
	~Shader();

	void bind() const;
	void unbind() const;


	//Note you need to bind program first to be able to set uniforms
	void setUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
	void setUniformVec4f(const std::string& name, glm::vec4 vec);
	void setUniformMat4(const std::string& name, glm::mat4 matrix);
	void setUniform1f(const std::string& name, float f0);
	void setUniform2f(const std::string& name, float f0, float f1);
	void setUniform1i(const std::string& name, int v);
	void setUniform1iv(const std::string& name,int count, int* v);




};

