#pragma once
#include "ndpch.h"
#include "graphics/API/Shader.h"
#define BREAK_IF_SHADER_COMPILE_ERROR 1

static const char* s_current_file = "null";

class GLShader:public Shader
{
private:
	unsigned int m_id;
	int getUniformLocation(const std::string& name);
	std::unordered_map<std::string, int> cache;
#ifdef ND_DEBUG
	bool m_isBound = false;
	void checkIsBound()
	{
		if (!m_isBound)
			ASSERT(false, "Cannot set uniform when no shader is bound");
	}
#define SHADER_CHECK_BOUNDIN checkIsBound()
#else
#define SHADER_CHECK_BOUNDIN 
#endif
public:
	GLShader(const ShaderProgramSources& src);
	GLShader(const std::string& file_path);
	virtual ~GLShader();

	void bind() const override;
	void unbind() const override;


	//Note you need to bind program first to be able to set uniforms
	void setUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
	void setUniformVec4f(const std::string& name, const glm::vec4& vec);
	void setUniformVec3f(const std::string& name, const glm::vec3& vec);
	void setUniformMat4(const std::string& name, const glm::mat4& matrix);
	void setUniform1f(const std::string& name, float f0);
	void setUniform2f(const std::string& name, float f0, float f1);
	void setUniform3f(const std::string& name, float f0, float f1,float f2);
	void setUniform1i(const std::string& name, int v);
	void setUniform1iv(const std::string& name, int count, int* v);
	inline void setUniformVec2f(const std::string& name, const glm::vec2& v)
	{
		setUniform2f(name, v.x, v.y);
	}
};
