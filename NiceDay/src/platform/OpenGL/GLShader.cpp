#include "ndpch.h"
#include "GLShader.h"
#include "GLRenderer.h"

static void shaderTypeToString(unsigned int t)
{
	switch (t)
	{
	case GL_VERTEX_SHADER:
		ND_ERROR("VERTEX");
		break;
	case GL_FRAGMENT_SHADER:
		ND_ERROR("FRAGMENT");
		break;
	case GL_GEOMETRY_SHADER:
		ND_ERROR("GEOMETRY");
		break;
	default:
		ND_ERROR("NONE");
		break;
	}
}


static GLShader::ShaderProgramSources parseShader(const std::string& file_path)
{
	s_current_file = file_path.c_str();
	
	ASSERT(std::filesystem::exists(s_current_file), "Invalid shader path {}",s_current_file);
	std::ifstream stream(file_path);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1,
		GEOMETRY = 2
	};

	ShaderType shaderType = ShaderType::NONE;
	std::string line;
	std::stringstream ss[3];
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				shaderType = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				shaderType = ShaderType::FRAGMENT;
			else if (line.find("geometry") != std::string::npos)
				shaderType = ShaderType::GEOMETRY;
		}
		else if (shaderType != ShaderType::NONE)
		{
			ss[(int)shaderType] << line << "\n";
		}
	}
	return {ss[0].str(), ss[1].str(), ss[2].str()};
}

static unsigned int compileShader(unsigned int type, const std::string& src)
{
	unsigned int id = glCreateShader(type);
	const char* sr = src.c_str();

	GLCall(glShaderSource(id, 1, &sr, nullptr));
	GLCall(glCompileShader(id));
	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		ND_ERROR("[Shader Error] Failed to compile shader {}", s_current_file);
		shaderTypeToString(type);
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)malloc(length * sizeof(char));
		GLCall(glGetShaderInfoLog(id, length, &length, message));
		ND_ERROR(message);
		free(message);
#if BREAK_IF_SHADER_COMPILE_ERROR == 1
		ASSERT(false, "Shader compile error");
#endif
		return 0;
	}
	return id;
}

static unsigned int buildProgram(const Shader::ShaderProgramSources& src)
{
	GLCall(unsigned int program = glCreateProgram());
	unsigned int vs = compileShader(GL_VERTEX_SHADER, src.vertexSrc);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, src.fragmentSrc);
	unsigned int gs = 0;
	bool geometry = !src.geometrySrc.empty();

	if (geometry)
	{
		gs = compileShader(GL_GEOMETRY_SHADER, src.geometrySrc);
		GLCall(glAttachShader(program, gs));
	}
	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));
	if (geometry)
		GLCall(glDeleteShader(gs));

	//ND_TRACE("[Shader: {}] parsed", program);

	return program;
}

GLShader::GLShader(const Shader::ShaderProgramSources& src) : m_id(0)
{
	m_id = buildProgram(src);
}

GLShader::GLShader(const std::string& file_path) : m_id(0)
{
	Shader::ShaderProgramSources& s = parseShader(ND_RESLOC(file_path));
	m_id = buildProgram(s);
}


void GLShader::bind() const
{
#ifdef ND_DEBUG
	const_cast<bool&>(m_isBound) = true;
#endif
	GLCall(glUseProgram(m_id));
}

void GLShader::unbind() const
{
#ifdef ND_DEBUG
	const_cast<bool&>(m_isBound) = false;
#endif
	GLCall(glUseProgram(0));
}

void GLShader::setUniformMat4(const std::string& name, const glm::mat4& matrix)
{
	SHADER_CHECK_BOUNDIN;
	GLCall(glUniformMatrix4fv(getUniformLocation(name), 1, false, glm::value_ptr(matrix)));
}

void GLShader::setUniform4f(const std::string& name, float f0, float f1, float f2, float f3)
{
	SHADER_CHECK_BOUNDIN;
	GLCall(glUniform4f(getUniformLocation(name), f0, f1, f2, f3));
}

void GLShader::setUniformVec4f(const std::string& name, const glm::vec4& vec)
{
	setUniform4f(name, vec[0], vec[1], vec[2], vec[3]);
}


void GLShader::setUniform1f(const std::string& name, float f0)
{
	SHADER_CHECK_BOUNDIN;
	GLCall(glUniform1f(getUniformLocation(name), f0));
}

void GLShader::setUniform2f(const std::string& name, float f0, float f1)
{
	SHADER_CHECK_BOUNDIN;
	GLCall(glUniform2f(getUniformLocation(name), f0, f1));
}

void GLShader::setUniform1i(const std::string& name, int v)
{
	SHADER_CHECK_BOUNDIN;
	GLCall(glUniform1i(getUniformLocation(name), v));
}

void GLShader::setUniform1iv(const std::string& name, int count, int* v)
{
	SHADER_CHECK_BOUNDIN;
	GLCall(glUniform1iv(getUniformLocation(name), count, v));
}


int GLShader::getUniformLocation(const std::string& name)
{
	if (cache.find(name) != cache.end())
	{
		return cache[name];
	}
	int out = glGetUniformLocation(m_id, name.c_str());
	if (out == -1)
	{
		ND_WARN("[Shader: {}], Uniform doesn't exist: {}", m_id, name);
		return -1;
	}

	cache[name] = out;
	return out;
}

GLShader::~GLShader()
{
	GLCall(glDeleteProgram(m_id));
}
