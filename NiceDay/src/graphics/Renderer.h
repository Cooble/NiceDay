#pragma once

#include "API/VertexArray.h"

#include "API/Shader.h"

class Sprite;
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
	void clear();
};
