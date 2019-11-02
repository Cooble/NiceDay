#include "ndpch.h"
#include "Sprite2D.h"
#include "platform/OpenGL/GLShader.h"


VertexBuffer* Sprite2D::s_vbo=nullptr;
VertexArray* Sprite2D::s_vao;
Shader* Sprite2D::s_program;

void Sprite2D::init()
{
	if (s_vbo)//dont want to reinit
		return;
	float quad[] = {
		1, 0,
		1, 1,
		0, 0,
		0, 1,
	};
	s_vbo = VertexBuffer::create(quad, sizeof(quad), BufferUsage::STATIC_DRAW);
	VertexBufferLayout l;
	l.push<float>(2);
	s_vao = VertexArray::create();
	s_vbo->setLayout(l);
	s_vao->addBuffer(*s_vbo);

	s_program = ShaderLib::loadOrGetShader("res/shaders/Sprite2D.shader");
	s_program->bind();
	dynamic_cast<GLShader*>(s_program)->setUniform1i("u_texture", 0);
	s_program->unbind();
}


void Sprite2D::computeModelMatrix()
{
	m_model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(
		m_position.x,
		m_position.y, 0.0f));
	m_model_matrix = glm::scale(m_model_matrix, glm::vec3(m_scale.x,m_scale.y, 1));
	m_stale_model_matrix = false;
}

void Sprite2D::computeUVMatrix()
{
	m_uv_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(
		m_cutout.x,
		m_cutout.y, 0.0f));
	m_uv_matrix = glm::scale(m_uv_matrix, glm::vec3(m_cutout.z, m_cutout.w, 1));
	m_stale_uv_matrix = false;
}

Sprite2D::Sprite2D(Texture* t)
	: m_texture(t), m_stale_model_matrix(false)
{

}

Sprite2D::Sprite2D(const char* texture_path)
	:Sprite2D(Texture::create(TextureInfo(texture_path)))
{

}

Sprite2D::~Sprite2D()
{
	delete m_texture;
}

void Sprite2D::setPosition(const glm::vec2& v)
{
	m_position = v;
	m_stale_model_matrix = true;
}

void Sprite2D::setScale(const glm::vec2& v)
{
	m_scale = v;
	m_stale_model_matrix = true;
}

void Sprite2D::setCutout(const glm::vec4& v)
{
	m_cutout = v;
	m_stale_uv_matrix = true;
}

void Sprite2D::setCutout()
{
	setCutout(glm::vec4(0, 0, 1, 1));
}

void Sprite2D::setDimensions(int pixelX, int pixelY)
{
	setScale(glm::vec2((float)pixelX / m_texture->getWidth(), (float)pixelY / m_texture->getHeight()));
}

const glm::mat4& Sprite2D::getModelMatrix()
{
	if (m_stale_model_matrix)
		computeModelMatrix();
	return m_model_matrix;
}

const glm::mat4& Sprite2D::getUVMatrix()
{
	if (m_stale_uv_matrix)
		computeUVMatrix();
	return m_uv_matrix;
}
