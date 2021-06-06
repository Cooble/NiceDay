#pragma once
namespace nd {

class Shader;
class VertexArray;
class Sprite;
class IndexBuffer;
class VertexBuffer;
class FrameBuffer;
class Texture;

enum class GraphicsAPI
{
	None = 0,
	OpenGL
};

class Renderer
{
private:
	static GraphicsAPI s_api;
	inline static FrameBuffer* s_default_fbo = nullptr;
public:
	inline static GraphicsAPI getAPI()
	{
		return s_api;
	}

	Renderer();
	~Renderer();

	static void setDefaultFBO(FrameBuffer* fbo) { s_default_fbo = fbo; }
	static FrameBuffer* getDefaultFBO() { return s_default_fbo; }
	void draw(const VertexArray& vao, const Shader& shader, const IndexBuffer& ibo);
	void clear();
};
}
