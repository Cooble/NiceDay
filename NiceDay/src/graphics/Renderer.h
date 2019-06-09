#pragma once
#include <glad/glad.h>

#include "buffer/VertexArray.h"
#include "Sprite2D.h"

#include "Shader.h"
#ifndef LOADED_CALL_G

class IndexBuffer;
class VertexBuffer;
//this enables glGetError()

void checkGLError(int line, const char* method_name, const char* file);

#ifdef ND_DEBUG
#define Call(x) \
	while (glGetError() != GL_NO_ERROR);\
	x;\
	checkGLError(__LINE__,#x,__FILE__);
#else
#define Call(x) x;
#endif // DEBUG

#define LOADED_CALL_G
#endif // LOADED_CALL_G

	
enum class GraphicsAPI
{
	None =0,
	OpenGL,
	Direct3D
	
};



class Renderer
{
private:
	static GraphicsAPI s_api;
public:
	inline static GraphicsAPI getAPI()
	{
		return s_api;
	}
	Renderer();
	~Renderer();

	void draw(const VertexArray& vao, const Shader& shader, const IndexBuffer& ibo);
	void draw(glm::mat4 trans,Sprite2D& sprite);
	void clear();
};
