#pragma once
#include "ndpch.h"
#include "graphics/API/Shader.h"

namespace nd::internal {

static const char* s_current_file = "null";

class GLShader : public Shader
{
private:
	ShaderLayout m_layout;
	unsigned int m_id;
	std::string m_file_path;
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
	~GLShader() override;

	void bind() const override;
	void unbind() const override;
	const ShaderLayout& getLayout() const override { return m_layout; }


	//Note you need to bind program first to be able to set uniforms
	void setUniform4f(const std::string& name, float f0, float f1, float f2, float f3);
	void setUniformVec4f(const std::string& name, const glm::vec4& vec);
	void setUniformVec3f(const std::string& name, const glm::vec3& vec);
	void setUniformMat4(const std::string& name, const glm::mat4& matrix);
	void setUniformMat4v(const std::string& name, int count, const glm::mat4* matrix);
	void setUniformMat3v(const std::string& name, int count, const glm::mat3* matrix);

	void setUniform1f(const std::string& name, float f0);
	void setUniform2f(const std::string& name, float f0, float f1);
	void setUniform3f(const std::string& name, float f0, float f1, float f2);
	void setUniform1i(const std::string& name, int v);

	void setUniform1iv(const std::string& name, int count, int* v);
	void setUniform1fv(const std::string& name, int count, float* v);

	void setUniformiv(const std::string& name, int count, int arraySize, int* v);
	void setUniformuiv(const std::string& name, int count, int arraySize, uint32_t* v);
	void setUniformfv(const std::string& name, int count, int arraySize, float* v);

	inline void setUniformVec2f(const std::string& name, const glm::vec2& v)
	{
		setUniform2f(name, v.x, v.y);
	}

	const std::string& getFilePath() const override { return m_file_path; }
};
}
