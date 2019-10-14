#include "ndpch.h"
#include "BatchRenderer2D.h"
#include "Renderable2D.h"
#include "API/Texture.h"
#include "GContext.h"

#define MAX_TEXTURES 16

#define MAX_QUADS 1000
#define MAX_VERTICES (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)




BatchRenderer2D::BatchRenderer2D()
{
#if !USE_MAP_BUF
	m_buff = new VertexData[MAX_VERTICES];
#endif
	m_transformation_stack.emplace_back(1.0f);
	m_back = m_transformation_stack[0];

	auto uniforms = new int[MAX_TEXTURES];
	for (int i = 0; i < MAX_TEXTURES; ++i)
	{
		uniforms[i] = i;
	}
	m_shader = new Shader("res/shaders/Sprite.shader");
	m_shader->bind();
	m_shader->setUniform1iv("u_textures", MAX_TEXTURES, uniforms);
	m_shader->setUniformMat4("u_projectionMatrix", mat4(1.0f));
	m_shader->unbind();
	delete[] uniforms;

	VertexBufferLayout l;
	l.push<float>(3);			//POS
	l.push<float>(2);			//UV
	l.push<unsigned int>(1);	//TEXTURE_SLOT
	
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

BatchRenderer2D::~BatchRenderer2D()
{
	delete m_vbo;
	delete m_vao;
	delete m_ibo;
#if !USE_MAP_BUF
	delete m_buff;
#endif
}

void BatchRenderer2D::push(const mat4& trans)
{
	m_transformation_stack.push_back(m_back * trans);
	m_back = m_transformation_stack[m_transformation_stack.size() - 1];
}

void BatchRenderer2D::pop()
{
	if (m_transformation_stack.size() > 1)
		m_transformation_stack.pop_back();
	m_back = m_transformation_stack[m_transformation_stack.size() - 1];
}

int BatchRenderer2D::bindTexture(const Texture* t)
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

void BatchRenderer2D::begin()
{
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
	glm::vec4 inV = { 0,0,0,1 };
	memcpy((char*)&inV, (char*)&v, sizeof(v));
	inV = m * inV;
	return *((glm::vec3*)&inV);
}

void BatchRenderer2D::submit(const Renderable2D& renderable)
{
	int textureSlot = bindTexture(renderable.getTexture());
	auto& pos = renderable.getPosition();
	auto& size = renderable.getSize();
	auto& uv = renderable.getUV();

	//m_back = glm::mat4(1.0f);

	m_vertex_data->position = (m_back) * pos;
	m_vertex_data->uv = uv.uv[0];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, 0, 0));
	m_vertex_data->uv = uv.uv[1];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, size.y, 0));
	m_vertex_data->uv = uv.uv[2];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(0, size.y, 0));
	m_vertex_data->uv = uv.uv[3];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_indices_count += 6;
	if (m_indices_count == MAX_INDICES)
	{
		flush();
		begin();
	}


}
void BatchRenderer2D::submit(const glm::vec3& pos,const glm::vec2& size,const UVQuad& uv,Texture* t)
{
	int textureSlot = bindTexture(t);
	

	m_vertex_data->position = (m_back) * pos;
	m_vertex_data->uv = uv.uv[0];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, 0, 0));
	m_vertex_data->uv = uv.uv[1];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, size.y, 0));
	m_vertex_data->uv = uv.uv[2];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(0, size.y, 0));
	m_vertex_data->uv = uv.uv[3];
	m_vertex_data->textureSlot = textureSlot;
	++m_vertex_data;

	m_indices_count += 6;
	if (m_indices_count == MAX_INDICES)
	{
		flush();
		begin();
	}


}

void BatchRenderer2D::flush()
{
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

	Gcon.enableBlend();
	Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
	Gcon.cmdDrawElements(Topology::TRIANGLES, m_indices_count);
}
