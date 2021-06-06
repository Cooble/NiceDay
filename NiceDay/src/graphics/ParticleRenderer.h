#pragma once
#include "API/Shader.h"
#include "API/VertexArray.h"


namespace nd {

//should use mapbuffer or subdata

#define USE_MAP_BUF 1


struct UVQuad;
class Texture;
class Renderable2D;
class FrameBuffer;
using namespace glm;


class ParticleRenderer
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
	std::vector<mat4> m_transformation_stack;
	std::vector<const Texture*> m_textures;
	mat4 m_back;
	ShaderPtr m_shader;
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;
	FrameBuffer* m_fbo;

#if !USE_MAP_BUF
		VertexData* m_buff;
#endif
	VertexData* m_vertex_data;
	int m_indices_count;
private:
	int bindTexture(const Texture* t);
public:
	ParticleRenderer();
	~ParticleRenderer();

	void push(const mat4& trans)
	{
		m_transformation_stack.push_back(m_back * trans);
		m_back = m_transformation_stack[m_transformation_stack.size() - 1];
	}

	void pop()
	{
		if (m_transformation_stack.size() > 1)
			m_transformation_stack.pop_back();
		m_back = m_transformation_stack[m_transformation_stack.size() - 1];
	}

	void begin(FrameBuffer* fbo = nullptr);
	void submit(const glm::vec3& pos, const glm::vec2& size, const UVQuad& uv0, const UVQuad& uv1, Texture* t,
	            float mix);
	void flush();
};
}
