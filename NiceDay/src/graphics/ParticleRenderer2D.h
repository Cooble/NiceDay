#pragma once
#include "API/Shader.h"
#include "API/VertexArray.h"

struct UVQuad;
class Texture;
class Renderable2D;
using namespace glm;


class ParticleRenderer2D
{
	struct VertexData
	{
		glm::vec3 position;
		glm::vec2 uv0;
		glm::vec2 uv1;
		int textureSlot;
		float mix;
	};
private:
	std::vector<mat3> m_transformation_stack;
	std::vector<const Texture*> m_textures;
	mat3 m_back;
	Shader* m_shader;
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;
	VertexData* m_buff;

	VertexData* m_vertex_data;
	int m_indices_count;
private:
	int bindTexture(const Texture* t);
public:
	ParticleRenderer2D();
	~ParticleRenderer2D();

	void push(const mat3& trans);
	void pop();

	void begin();
	void submit(const vec2& pos, const vec2& size, const UVQuad& uv0, const UVQuad& uv1, Texture* t, float mix);
	void flush();


};
