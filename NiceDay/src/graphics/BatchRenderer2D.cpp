#include "ndpch.h"
#include "BatchRenderer2D.h"
#include "Renderable2D.h"
#include "API/Texture.h"
#include "API/FrameBuffer.h"
#include "GContext.h"
#include "font/TextBuilder.h"
#include "FontMaterial.h"
#include "platform/OpenGL/GLShader.h"
#include "core/Core.h"

namespace nd {

constexpr int MAX_TEXTURES = 16;
constexpr int MAX_QUADS = 5000;
constexpr int MAX_VERTICES = MAX_QUADS * 4;
constexpr int MAX_INDICES = MAX_QUADS * 6;

BatchRenderer2D::BatchRenderer2D()
{
	m_transformation_stack.emplace_back(1.0f);
	m_back = m_transformation_stack[0];

	prepareQuad();
	prepareText();
}

void BatchRenderer2D::prepareQuad()
{
#if !USE_MAP_BUF
	m_buff = new VertexData[MAX_VERTICES];
#endif

	auto uniforms = new int[MAX_TEXTURES];
	for (int i = 0; i < MAX_TEXTURES; ++i)
	{
		uniforms[i] = i;
	}
	m_shader = ShaderLib::loadOrGetShader("res/shaders/Sprite.shader");
	m_shader->bind();
	std::static_pointer_cast<nd::internal::GLShader>(m_shader)->setUniform1iv("u_textures", MAX_TEXTURES, uniforms);
	std::static_pointer_cast<nd::internal::GLShader>(m_shader)->setUniformMat4("u_projectionMatrix", mat4(1.0f));
	m_shader->unbind();
	delete[] uniforms;

	VertexBufferLayout l{
		g_typ::VEC3, //POS
		g_typ::VEC2, //UV
		g_typ::UNSIGNED_INT, //TEXTURE_SLOT
		g_typ::UNSIGNED_INT, //COLOR

	};


	m_vbo = VertexBuffer::create(nullptr, MAX_VERTICES * sizeof(VertexData), BufferUsage::STREAM_DRAW);
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

void BatchRenderer2D::prepareText()
{
#if !USE_MAP_BUF
	m_text_buff = new TextVertexData[MAX_VERTICES];
#endif

	m_text_shader = ShaderLib::loadOrGetShader("res/shaders/Font.shader");
	m_text_shader->bind();
	std::static_pointer_cast<nd::internal::GLShader>(m_text_shader)->setUniform1i("u_texture", 0);
	std::static_pointer_cast<nd::internal::GLShader>(m_text_shader)->setUniformMat4("u_transform", mat4(1.0f));
	m_text_shader->unbind();

	VertexBufferLayout l{
		g_typ::VEC3, //POS
		g_typ::VEC2, //UV
		g_typ::UNSIGNED_INT, //COLOR
		g_typ::UNSIGNED_INT, //BORDER_COLOR

	};

	m_text_vbo = VertexBuffer::create(nullptr, MAX_VERTICES * sizeof(TextVertexData), BufferUsage::STREAM_DRAW);
	m_text_vbo->setLayout(l);

	m_text_vao = VertexArray::create();
	m_text_vao->addBuffer(*m_text_vbo);
}


BatchRenderer2D::~BatchRenderer2D()
{
	delete m_vbo;
	delete m_vao;
	delete m_ibo;
#if !USE_MAP_BUF
	delete m_buff;
#endif

	delete m_text_vbo;
	delete m_text_vao;
#if !USE_MAP_BUF
	delete m_text_buff;
#endif
}

void BatchRenderer2D::push(const mat4& trans)
{
	ASSERT(m_transformation_stack.size() < 500, "Somebody forget to call pop()..");
	m_transformation_stack.push_back(m_back * trans);
	m_back = m_transformation_stack[m_transformation_stack.size() - 1];
}

void BatchRenderer2D::pop(int count)
{
	for (int i = 0; i < count && m_transformation_stack.size() > 1; ++i)
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

	flushQuad();
	beginQuad();
	return bindTexture(t);
}


void BatchRenderer2D::begin(FrameBuffer* fbo)
{
	if (fbo)
		m_fbo = fbo;
	ASSERT(m_fbo, "Renderer Target must be specified");
	beginQuad();
	beginText();
}

void BatchRenderer2D::beginText()
{
	m_text_indices_count = 0;
#if USE_MAP_BUF
		m_vbo->bind();
		m_vertex_data = (VertexData*)m_vbo->mapPointer();
#else
	m_text_vertex_data = m_text_buff;
#endif
}

void BatchRenderer2D::flushText()
{
	if (m_text_indices_count == 0)
		return;
#if USE_MAP_BUF
		m_text_vbo->unMapPointer();
#else
	m_text_vbo->bind();
	m_text_vbo->changeData((char*)m_text_buff, sizeof(TextVertexData) * MAX_VERTICES, 0);
#endif
	m_text_vao->bind();
	m_ibo->bind();
	m_text_shader->bind();

	if (m_apply_default_blending)
	{
		Gcon.enableBlend();
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
	}
	static int BUF_S = 500;
	static auto lengths = new int[BUF_S];
	static auto indices = new uint64_t[BUF_S];
	memset(lengths, 0, BUF_S * sizeof(int));
	memset(indices, 0, BUF_S * sizeof(int) * 2);
	for (auto& fontMat : m_fonts)
	{
		if (fontMat.second.empty())
			continue;
		fontMat.first->texture->bind(0);
		//std::static_pointer_cast<nd::internal::GLShader>(m_text_shader)->setUniformVec4f("u_textColor", fontMat.first->color);
		//std::static_pointer_cast<nd::internal::GLShader>(m_text_shader)->setUniformVec4f("u_borderColor", fontMat.first->border_color);

		if (fontMat.second.size() > BUF_S)
		{
			delete[] lengths;
			delete[] indices;
			BUF_S = fontMat.second.size();
			lengths = new int[BUF_S];
			indices = new uint64_t[BUF_S];
		}

		for (int i = 0; i < fontMat.second.size(); ++i)
		{
			lengths[i] = fontMat.second[i].length;
			indices[i] = fontMat.second[i].fromIndex * sizeof(int);
		}
		Gcon.cmdDrawMultiElements(Topology::TRIANGLES, reinterpret_cast<uint32_t*>(indices), lengths,
		                          fontMat.second.size());
		fontMat.second.clear();
	}
}

void BatchRenderer2D::beginQuad()
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

void BatchRenderer2D::flushQuad()
{
	if (m_indices_count == 0)
		return;
#if USE_MAP_BUF
		m_vbo->unMapPointer();
#else
	m_vbo->bind();
	m_vbo->changeData((char*)m_buff, sizeof(VertexData) * MAX_VERTICES, 0);
	//m_vbo->changeData((char*)m_buff, sizeof(VertexData)*(m_indices_count/6)*4, 0);
#endif
	m_vao->bind();
	m_ibo->bind();
	m_shader->bind();

	for (int i = 0; i < m_textures.size(); ++i)
		m_textures[i]->bind(i);

	if (m_apply_default_blending)
	{
		Gcon.enableBlend();
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
	}
	Gcon.cmdDrawElements(Topology::TRIANGLES, m_indices_count);
}

static glm::vec3 operator*(const glm::mat4& m, const glm::vec3& v)
{
	glm::vec4 inV = {v.x, v.y, v.z, 1};
	inV = m * inV;
	return *((glm::vec3*)&inV);
}

void BatchRenderer2D::submit(const Renderable2D& renderable)
{
	auto& pos = renderable.getPosition();
	auto& size = renderable.getSize();
	auto& uv = renderable.getUV();


	submitTextureQuad(pos, size, uv, renderable.getTexture());
}

void BatchRenderer2D::submitTextureQuad(const glm::vec3& pos, const glm::vec2& size, const UVQuad& uv, const Texture* t,
                                        float alpha)
{
	int textureSlot = bindTexture(t);
	auto colo = ((int)(alpha * 255) << 24);

	m_vertex_data->position = (m_back) * pos;
	m_vertex_data->uv = uv.uv[0];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->color = colo;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, 0, 0));
	m_vertex_data->uv = uv.uv[1];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->color = colo;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, size.y, 0));
	m_vertex_data->uv = uv.uv[2];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->color = colo;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(0, size.y, 0));
	m_vertex_data->uv = uv.uv[3];
	m_vertex_data->textureSlot = textureSlot;
	m_vertex_data->color = colo;
	++m_vertex_data;

	m_indices_count += 6;
	if (m_indices_count == MAX_INDICES)
	{
		flush();
		begin(m_fbo);
	}
}

void BatchRenderer2D::submitColorQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color)
{
	uint32_t col = (
		((int)(color.r * 255) << 0) |
		((int)(color.g * 255) << 8) |
		((int)(color.b * 255) << 16) |
		((int)(color.a * 255) << 24));
	m_vertex_data->position = (m_back) * pos;
	m_vertex_data->textureSlot = std::numeric_limits<int>::max();
	m_vertex_data->color = col;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, 0, 0));
	m_vertex_data->textureSlot = std::numeric_limits<int>::max();
	m_vertex_data->color = col;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(size.x, size.y, 0));
	m_vertex_data->textureSlot = std::numeric_limits<int>::max();
	m_vertex_data->color = col;
	++m_vertex_data;

	m_vertex_data->position = (m_back) * (pos + vec3(0, size.y, 0));
	m_vertex_data->textureSlot = std::numeric_limits<int>::max();
	m_vertex_data->color = col;
	++m_vertex_data;

	m_indices_count += 6;
	if (m_indices_count == MAX_INDICES)
	{
		flushQuad();
		beginQuad();
	}
}

void BatchRenderer2D::submitText(const TextMesh& mesh, const FontMaterial* material)
{
	if (mesh.getVertexCount() == 0)
		return;
	auto data = mesh.getSrc();

	if (m_text_indices_count + mesh.currentCharCount * 6 >= MAX_INDICES)
	{
		flushText();
		beginText();
	}
	m_fonts[material].push_back({m_text_indices_count, mesh.currentCharCount * 6});
	for (int i = 0; i < mesh.currentCharCount; ++i)
	{
		const auto& ch = data[i];

		const auto& v00 = ch.vertex[0];
		const auto& v01 = ch.vertex[1];
		const auto& v02 = ch.vertex[2];
		const auto& v03 = ch.vertex[3];

		auto& v0 = (m_text_vertex_data + 0)->position;
		auto& v1 = (m_text_vertex_data + 1)->position;
		auto& v2 = (m_text_vertex_data + 2)->position;
		auto& v3 = (m_text_vertex_data + 3)->position;

		v0 = glm::vec3(v00.x, v00.y, 0);
		v1 = glm::vec3(v01.x, v01.y, 0);
		v2 = glm::vec3(v02.x, v02.y, 0);
		v3 = glm::vec3(v03.x, v03.y, 0);

		v0 = m_back * v0;
		//v1 = m_back * v1;
		v2 = m_back * v2;
		//v3 = m_back * v3;


		v1 = glm::vec3(v2.x, v0.y, v0.z);
		v3 = glm::vec3(v0.x, v2.y, v0.z);

		(m_text_vertex_data + 0)->uv = v00.uv;
		(m_text_vertex_data + 1)->uv = v01.uv;
		(m_text_vertex_data + 2)->uv = v02.uv;
		(m_text_vertex_data + 3)->uv = v03.uv;

		(m_text_vertex_data + 0)->color = v00.color;
		(m_text_vertex_data + 1)->color = v00.color;
		(m_text_vertex_data + 2)->color = v00.color;
		(m_text_vertex_data + 3)->color = v00.color;

		(m_text_vertex_data + 0)->borderColor = v00.borderColor;
		(m_text_vertex_data + 1)->borderColor = v00.borderColor;
		(m_text_vertex_data + 2)->borderColor = v00.borderColor;
		(m_text_vertex_data + 3)->borderColor = v00.borderColor;


		m_text_vertex_data += 4;
		m_text_indices_count += 6;
	}
}

void BatchRenderer2D::flush()
{
	m_fbo->bind();
	flushQuad();
	flushText();
}
}
