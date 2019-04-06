#include "ndpch.h"
#include "Program.h"
#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>



struct ShaderProgramSources
{
	std::string vertexSrc;
	std::string fragmentSrc;

};

static ShaderProgramSources parseShader(const std::string &file_path) {
	
	std::ifstream stream(file_path);

	enum class ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	ShaderType shaderType = ShaderType::NONE;
	std::string line;
	std::stringstream ss[2];
	while (getline(stream, line)) {
		if (line.find("#shader")!= std::string::npos) {
			if (line.find("vertex") != std::string::npos)
				shaderType = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos) 
				shaderType = ShaderType::FRAGMENT;

			
		}
		else if(shaderType!=ShaderType::NONE){
			//std::cout << (int)shaderType<<" ";
			//std::cout << line << std::endl;
			ss[(int)shaderType] << line << "\n";

		}



	}
	return { ss[0].str(),ss[1].str()};

}

static unsigned int compileShader(unsigned int type, const std::string& src) {
	unsigned int id = glCreateShader(type);
	const char* sr = src.c_str();

	Call(glShaderSource(id, 1, &sr, nullptr));
	Call(glCompileShader(id));
	int result;
	Call(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		std::cout << "Failed to compile shader";

		int length;
		Call(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char * message = (char*)alloca(length * sizeof(char));
		Call(glGetShaderInfoLog(id, length, &length, message));
		std::cout << message << std::endl;
		return 0;

	}
	return id;

	//TODO errors?

}

static unsigned int buildProgram(const std::string &vertex, const std::string &fragment) {
	Call(unsigned int program = glCreateProgram());
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertex);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragment);
	Call(glAttachShader(program, vs));
	Call(glAttachShader(program, fs));
	Call(glLinkProgram(program));
	Call(glValidateProgram(program));

	Call(glDeleteShader(vs));
	Call(glDeleteShader(fs));
	std::cout << "Parsed program with id: " << program << std::endl;

	return program;
}

Program::Program(const std::string &vertex, const std::string &fragment) :m_id(0) {
	m_id = buildProgram(vertex, fragment);
}
Program::Program(const std::string &file_path):m_id(0) {
	ShaderProgramSources s = parseShader(file_path);
	m_id = buildProgram(s.vertexSrc, s.fragmentSrc);
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
void Program::setUniform2f(const std::string & name, float f0,float f1)
{
	glUniform2f(getUniformLocation(name), f0,f1);
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
	int out =  glGetUniformLocation(m_id, name.c_str());
	if (out == -1) 
		std::cout << "Uniform doesn't exist: " << name << std::endl;

	cache[name] = out;
	return out;
}

Program::~Program()
{
	glDeleteProgram(m_id);
}
