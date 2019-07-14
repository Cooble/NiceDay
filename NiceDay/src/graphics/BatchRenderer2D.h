#pragma once
#include "Shader.h"
#include "buffer/VertexArray.h"


//should use mapbuffer or subdata
#define USE_MAP_BUF 1

class Texture;
class Renderable2D;
using namespace glm;

struct VertexData
{
	glm::vec3 position;
	glm::vec2 uv;
	int textureSlot;
};
class BatchRenderer2D
{
private:
	std::vector<mat4> m_transformation_stack;
	std::vector<const Texture*> m_textures;
	mat4 m_back;
	Shader* m_shader;
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;

#if !USE_MAP_BUF
	VertexData* m_buff;
#endif
	VertexData* m_vertex_data;
	int m_indices_count;
private:
	int bindTexture(const Texture* t);
public:
	BatchRenderer2D();
	~BatchRenderer2D();

	void push(const mat4& trans);
	void pop();

	void begin();
	void submit(const Renderable2D&);
	void flush();


};
