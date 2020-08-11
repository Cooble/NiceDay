#include "ndpch.h"
#include "ParticleRenderer.h"
#include "Renderable2D.h"
#include "API/Texture.h"
#include "platform/OpenGL/GLRenderer.h"
#include "API/Shader.h"
#include "API/Buffer.h"
#include "API/FrameBuffer.h"
#include "API/VertexArray.h"
#include "platform/OpenGL/GLShader.h"

#define MAX_TEXTURES 16

#define MAX_QUADS 10000
#define MAX_VERTICES (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)




ParticleRenderer::ParticleRenderer()
{
#if !USE_MAP_BUF
	m_buff = new ParticleBatchRenderer2D::VertexData[MAX_VERTICES];
#endif
	m_transformation_stack.push_back(glm::mat4(1.0f));
	m_back = m_transformation_stack[0];

	auto uniforms = new int[MAX_TEXTURES];
	for (int i = 0; i < MAX_TEXTURES; ++i)
	{
		uniforms[i] = i;
	}
	m_shader = ShaderLib::loadOrGetShader("res/shaders/ParticleSprite.shader");
	m_shader->bind();
	std::static_pointer_cast<GLShader>(m_shader)->setUniform1iv("u_textures", MAX_TEXTURES, uniforms);
	std::static_pointer_cast<GLShader>(m_shader)->setUniformMat4("u_projectionMatrix", mat4(1.0f));
	m_shader->unbind();
	delete[] uniforms;

	VertexBufferLayout l;
	l.push<float>(3);			//POS
	l.push<float>(2);			//UV0
	l.push<float>(2);			//UV1
	l.push<unsigned int>(1);	//TEXTURE_SLOT
	l.push<float>(1);			//MIX_CONSTANT
	
	m_vbo = VertexBuffer::create(nullptr, MAX_VERTICES*sizeof(VertexData), BufferUsage::STREAM_DRAW);
	m_vbo->setLayout(l);

	m_vao = VertexArray::create();
	m_vao->addBuffer(*m_vbo);

	auto indices = new uint32_t[MAX_INDICES]; //use shorts instead
	uint32_t offset = 0;
	for (int i = 0; i < MAX_INDICES; i += 6)
	{
		indices[i + 0] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;

		indices[i + 3] = offset + 0;
		indices[i + 4] = offset + 2;
		indices[i + 5] = offset + 3;

		offset += 4;
	}
	m_ibo = IndexBuffer::create(indices, MAX_INDICES);
	delete[] indices;
}

ParticleRenderer::~ParticleRenderer()
{
	delete m_vbo;
	delete m_vao;
	delete m_ibo;
#if !USE_MAP_BUF
	delete m_buff;
#endif
}

int ParticleRenderer::bindTexture(const Texture* t)
{
	for (int i = 0; i < m_textures.size(); ++i)
		if (t == m_textures[i])
			return i;

	if (m_textures.size() != MAX_TEXTURES)
	{
		m_textures.push_back(t);
		return m_textures.size() - 1;
	}

	flush();
	begin();
	return bindTexture(t);
}

void ParticleRenderer::begin(FrameBuffer* fbo)
{
	if(fbo)
		m_fbo = fbo;
	ASSERT(m_fbo, "Renderer Target must be specified");
	m_indices_count = 0;
	m_textures.clear();
#if USE_MAP_BUF
	m_vbo->bind();
	m_vertex_data = (VertexData*)m_vbo->mapPointer();
#else
	m_vertex_data = m_buff;
#endif
	

}

static glm::vec3 operator*(const glm::mat4& m, const glm::vec3& v)
{
	glm::vec4 inV = *((glm::vec4*)&v);
	inV.w = 1;
	glm::vec4 ss = m * inV;
	return *((glm::vec3*)&ss);
}

void ParticleRenderer::submit(const glm::vec3& pos,const glm::vec2& size,const UVQuad& uv0, const UVQuad& uv1,Texture* t,float mix)
{
	int textureSlot = bindTexture(t);
	
	m_vertex_data->position = (m_back) * pos;
	m_vertex_data->uv0 = uv0.uv[0];
	m_vertex_data->uv1 = uv1.uv[0];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->mix = mix;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, 0, 0));
	m_vertex_data->uv0 = uv0.uv[1];
	m_vertex_data->uv1 = uv1.uv[1];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->mix = mix;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, size.y, 0));
	m_vertex_data->uv0 = uv0.uv[2];
	m_vertex_data->uv1 = uv1.uv[2];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->mix = mix;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(0, size.y, 0));
	m_vertex_data->uv0 = uv0.uv[3];
	m_vertex_data->uv1 = uv1.uv[3];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->mix = mix;
	++m_vertex_data;

	m_indices_count += 6;
	if (m_indices_count == MAX_INDICES)
	{
		flush();
		begin();
	}


}

void ParticleRenderer::flush()
{
	m_fbo->bind();
	
#if USE_MAP_BUF
	m_vbo->unMapPointer();
#else
	m_vbo->bind();
	m_vbo->changeData((char*)m_buff, sizeof(VertexData)*MAX_VERTICES, 0);
	//m_vbo->changeData((char*)m_buff, sizeof(VertexData)*(m_indices_count/6)*4, 0);
#endif
	m_vao->bind();
	m_ibo->bind();
	m_shader->bind();

	for (int i = 0; i < m_textures.size(); ++i)
		m_textures[i]->bind(i);

	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, nullptr));

}
