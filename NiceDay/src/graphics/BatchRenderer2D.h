#pragma once
#include "API/Shader.h"
#include "API/VertexArray.h"
#include "font/FontParser.h"

namespace nd {

//should use mapbuffer or subdata
//Disclaimer :::!!!! this should never be set to 1!!!!! (mapping to multiple vbos at once is not good)
#define USE_MAP_BUF 0

class FrameBuffer;
struct FontMaterial;
class TextMesh;
struct UVQuad;
class Texture;
class Renderable2D;
using namespace glm;


struct VertexData
{
	glm::vec3 position;
	glm::vec2 uv;
	uint32_t textureSlot;
	uint32_t color;
};

struct TextVertexData
{
	glm::vec3 position;
	glm::vec2 uv;
	uint32_t color;
	uint32_t borderColor;
};

class BatchRenderer2D
{
private:
	std::vector<mat4> m_transformation_stack;
	std::vector<const Texture*> m_textures;

	struct TextIBOView
	{
		int fromIndex;
		int length;
	};

	defaultable_map_other<const FontMaterial*, std::vector<TextIBOView>> m_fonts;
	mat4 m_back;
	ShaderPtr m_shader;
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
	IndexBuffer* m_ibo;

	ShaderPtr m_text_shader;
	VertexArray* m_text_vao;
	VertexBuffer* m_text_vbo;
	FrameBuffer* m_fbo = nullptr;

#if !USE_MAP_BUF
	VertexData* m_buff;
	TextVertexData* m_text_buff;
#endif
	VertexData* m_vertex_data;
	int m_indices_count;
	TextVertexData* m_text_vertex_data;
	int m_text_indices_count;
	//if src_alpha + 1-src_alpha should be used for each flush
	bool m_apply_default_blending = true;
private:
	int bindTexture(const Texture* t);
	void prepareQuad();
	void prepareText();

	void beginText();
	void flushText();
	void beginQuad();
	void flushQuad();

public:
	BatchRenderer2D();

	~BatchRenderer2D();

	void push(const mat4& trans);
	void pop(int count = 1);

	void setDefaultBlending(bool b) { m_apply_default_blending = b; }
	void begin(FrameBuffer* fbo);
	void submit(const Renderable2D& ren);
	void submitTextureQuad(const glm::vec3& pos, const glm::vec2& size, const UVQuad& uv, const Texture* t,
	                       float alpha = 1);
	void submitColorQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
	void submitColorTriangle(const glm::vec2& pos0,const glm::vec2& pos1,const glm::vec2& pos2, float z, const glm::vec4& color);

	void submitText(const TextMesh& mesh, const FontMaterial* material);
	void flush();
};
}
