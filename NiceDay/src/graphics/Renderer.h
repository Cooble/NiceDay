#pragma once

#include "buffer/VertexArray.h"
#include "Sprite2D.h"

#include "Shader.h"

class IndexBuffer;
class VertexBuffer;
	
enum class GraphicsAPI
{
	None = 0,
	OpenGL
	
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
