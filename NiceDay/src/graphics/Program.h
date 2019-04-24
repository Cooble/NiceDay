#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


class Program
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
	Program(const ShaderProgramSources& src);
	Program(const std::string &file_path);
	~Program();

	void bind() const;
	void unbind() const;


	//Note you need to bind program first to be able to set uniforms
	void setUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
	void setUniformVec4f(const std::string& name, glm::vec4 vec);
	void setUniformMat4(const std::string& name, glm::mat4 matrix);
	void setUniform1f(const std::string& name, float f0);
	void setUniform2f(const std::string& name, float f0, float f1);
	void setUniform1i(const std::string& name, int v);




};

