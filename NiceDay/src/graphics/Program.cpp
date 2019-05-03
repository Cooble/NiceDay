#include "ndpch.h"
#include "Program.h"
#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>


#define FORGET_BIND_PROTECTION //just bind it please, dont waste your time 


static void shaderTypeToString(unsigned int t)
{	
	if (t == GL_VERTEX_SHADER)
		ND_ERROR( "VERTEX");
	else if (t == GL_FRAGMENT_SHADER)
		ND_ERROR("FRAGMENT");
	else if (t == GL_GEOMETRY_SHADER)
		ND_ERROR("GEOMETRY");
	else
		ND_ERROR("NONE");

}


static Program::ShaderProgramSources parseShader(const std::string &file_path) {
	s_current_file = file_path.c_str();
	std::ifstream stream(file_path);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2
	};
	
	ShaderType shaderType = ShaderType::NONE;
	std::string line;
	std::stringstream ss[3];
	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos)
				shaderType = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				shaderType = ShaderType::FRAGMENT;
			else if (line.find("geometry") != std::string::npos)
				shaderType = ShaderType::GEOMETRY;


		}
		else if (shaderType != ShaderType::NONE) {
			ss[(int)shaderType] << line << "\n";

		}



	}
	return { ss[0].str(),ss[1].str(),ss[2].str() };

}

static unsigned int compileShader(unsigned int type, const std::string& src) {
	unsigned int id = glCreateShader(type);
	const char* sr = src.c_str();

	Call(glShaderSource(id, 1, &sr, nullptr));
	Call(glCompileShader(id));
	int result;
	Call(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		ND_ERROR("[Shader Error] Failed to compile shader {}",s_current_file);
		shaderTypeToString(type);
		int length;
		Call(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char * message = (char*)alloca(length * sizeof(char));
		Call(glGetShaderInfoLog(id, length, &length, message));
		ND_ERROR(message);
		return 0;

	}
	return id;
}

static unsigned int buildProgram(const Program::ShaderProgramSources& src) {
	Call(unsigned int program = glCreateProgram());
	unsigned int vs = compileShader(GL_VERTEX_SHADER, src.vertexSrc);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, src.fragmentSrc);
	unsigned int gs=0;
	bool geometry = src.geometrySrc != "";

	if (geometry) {
		gs = compileShader(GL_GEOMETRY_SHADER, src.geometrySrc);
		Call(glAttachShader(program, gs));
	}
	Call(glAttachShader(program, vs));
	Call(glAttachShader(program, fs));
	Call(glLinkProgram(program));
	Call(glValidateProgram(program));

	Call(glDeleteShader(vs));
	Call(glDeleteShader(fs));
	if (geometry)
		Call(glDeleteShader(gs));

	ND_TRACE("Parsed program with id: {}", program);

	return program;
}

Program::Program(const Program::ShaderProgramSources& src) :m_id(0) {
	m_id = buildProgram(src);
}

Program::Program(const std::string &file_path) : m_id(0) {
	Program::ShaderProgramSources& s = parseShader(file_path);
	m_id = buildProgram(s);
}

void Program::bind() const {
	Call(glUseProgram(m_id));
}

void Program::unbind() const {
	glUseProgram(0);
}

void Program::setUniformMat4(const std::string& name, glm::mat4 matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, false, glm::value_ptr(matrix));
}

void Program::setUniform4f(const std::string& name, float f0, float f1, float f2, float f3)
{
	glUniform4f(getUniformLocation(name), f0, f1, f2, f3);
}
void Program::setUniformVec4f(const std::string& name, glm::vec4 vec) {
	setUniform4f(name, vec[0], vec[1], vec[2], vec[3]);
}


void Program::setUniform1f(const std::string & name, float f0)
{
	glUniform1f(getUniformLocation(name), f0);
}
void Program::setUniform2f(const std::string & name, float f0, float f1)
{
	glUniform2f(getUniformLocation(name), f0, f1);
}

void Program::setUniform1i(const std::string & name, int v)
{
	glUniform1i(getUniformLocation(name), v);

}

int Program::getUniformLocation(const std::string& name)
{
	if (cache.find(name) != cache.end()) {
		return cache[name];
	}
	int out = glGetUniformLocation(m_id, name.c_str());
	if (out == -1)
		ND_WARN("Uniform doesn't exist: {}", name);

	cache[name] = out;
	return out;
}

Program::~Program()
{
	glDeleteProgram(m_id);
}
