#include "ndpch.h"
#include "Effect.h"
#include "platform/OpenGL/GLRenderer.h"
#include "GContext.h"
#include "core/App.h"

namespace nd {

void Effect::renderDefaultVAO()
{
	//Gcon.enableCullFace(false);

	s_vao->bind();
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void Effect::render(const Texture* t, FrameBuffer* fbo)
{
	fbo->bind();
	s_shader->bind();
	t->bind(0);
	renderDefaultVAO();
}

FrameBufferTexturePair::FrameBufferTexturePair(Texture* t)
	: m_texture(nullptr)
{
	m_fbo = FrameBuffer::create();
	replaceTexture(t);
}

FrameBufferTexturePair::FrameBufferTexturePair(): m_texture(nullptr)
{
	m_fbo = FrameBuffer::create();
}

FrameBufferTexturePair::~FrameBufferTexturePair()
{
	delete m_fbo;
	if (m_texture)
		delete m_texture;
}

void FrameBufferTexturePair::bind() const
{
	ASSERT(m_texture, "Cannot bind fbo, without texture attachment!");
	m_fbo->bind();
	Gcon.setViewport(m_texture->width(), m_texture->height());
}

void FrameBufferTexturePair::unbind() const
{
	m_fbo->unbind();
	Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());
}

SingleTextureEffect::SingleTextureEffect(const TextureInfo& targetTexture)
{
	m_output_texture = Texture::create(targetTexture);
	m_fbo = FrameBuffer::create();
	m_fbo->attachTexture(m_output_texture->getID(), 0);
}

SingleTextureEffect::~SingleTextureEffect()
{
	delete m_output_texture;
	delete m_fbo;
}

void SingleTextureEffect::defaultBind()
{
	m_fbo->bind();
	Gcon.setViewport(m_output_texture->width(), m_output_texture->height());
}

void SingleTextureEffect::defaultUnbind()
{
	m_fbo->unbind();
	Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());
}

AlphaMaskEffect::AlphaMaskEffect(const TextureInfo& targetTexture)
	: SingleTextureEffect(targetTexture.copy().format(TextureFormat::RGB))
{
}

void AlphaMaskEffect::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	t->bind(0);

	if (toFBO)
	{
		m_fbo->bind();
		Gcon.clear(BuffBit::COLOR);
		Gcon.disableBlend();
		Gcon.setViewport(m_output_texture->width(), m_output_texture->height());
		Effect::renderDefaultVAO();
		m_fbo->unbind();
		Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());
	}
	else
		Effect::renderDefaultVAO();
}

ScaleEdgesEffect::ScaleEdgesEffect(const TextureInfo& targetTexture)
	: SingleTextureEffect(targetTexture)
{
}

void ScaleEdgesEffect::render(const Texture* t, float scale, bool toFBO)
{
	getShader()->bind();
	std::static_pointer_cast<internal::GLShader>(getShader())->setUniform1f("u_scale", scale);
	t->bind(0);

	if (toFBO)
	{
		m_fbo->bind();
		Gcon.clear(BuffBit::COLOR);
		Gcon.disableBlend();
		Gcon.setViewport(m_output_texture->width(), m_output_texture->height());
		Effect::renderDefaultVAO();
		m_fbo->unbind();
		Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());
	}
	else
		Effect::renderDefaultVAO();
	getShader()->unbind();
}

GreenFilter::GreenFilter(const TextureInfo& targetTexture)
	: SingleTextureEffect(targetTexture)
{
}

void GreenFilter::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	t->bind(0);

	if (toFBO)
	{
		m_fbo->bind();
		Gcon.setViewport(m_output_texture->width(), m_output_texture->height());
		Effect::renderDefaultVAO();
		m_fbo->unbind();
		Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());
	}
	else
		Effect::renderDefaultVAO();
}

FrameBufferPingPong::FrameBufferPingPong(const TextureInfo& info)
{
	for (int i = 0; i < 2; ++i)
	{
		m_textures[i] = Texture::create(info);
		m_fbos[i] = FrameBuffer::create();
		m_fbos[i]->bind();
		m_fbos[i]->attachTexture(m_textures[i]->getID(), 0);
		m_fbos[i]->unbind();
	}
}

Texture* FrameBufferPingPong::getOutputTexture()
{
	return m_textures[m_currentRenderTarget];
}

void FrameBufferPingPong::bind()
{
	m_fbos[m_currentRenderTarget]->bind();
	Gcon.setViewport(m_textures[0]->width(), m_textures[0]->height());

#ifdef ND_DEBUG
	isBounded = true;
#endif
}

void FrameBufferPingPong::flip()
{
#ifdef ND_DEBUG
	ASSERT(isBounded, "Framebuffer Must be bounded!");
#endif
	m_currentRenderTarget = (m_currentRenderTarget + 1) & 1;
	m_fbos[m_currentRenderTarget]->bind();
}

void FrameBufferPingPong::unbind()
{
	m_fbos[m_currentRenderTarget]->unbind();
	Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());

#ifdef ND_DEBUG
	isBounded = false;
#endif
}

void Effecto::Blurer::blur(FrameBufferPingPong& fbos, Texture* input, int repeats)
{
	getShader()->bind();
	std::static_pointer_cast<internal::GLShader>(getShader())->setUniform2f("u_pixel_size", 1.f / input->width(),
	                                                                        1.f / input->height());
	Effect::getDefaultVAO().bind();
	fbos.bind();

	for (int i = 0; i < repeats * 2; ++i)
	{
		bool horizontal = i & 1;
		std::static_pointer_cast<internal::GLShader>(getShader())->setUniform1i("u_horizontal", horizontal);

		input->bind(0);

		Gcon.cmdDrawArrays(Topology::TRIANGLE_STRIP, 4);
		input = fbos.getOutputTexture();

		if (i != repeats * 2 - 1)
			fbos.flip();
	}
	fbos.unbind();
}

HorizontalBlur::HorizontalBlur(const TextureInfo& targetTexture)
	: SingleTextureEffect(targetTexture)
{
}

void HorizontalBlur::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	std::static_pointer_cast<internal::GLShader>(getShader())->setUniform2f(
		"u_pixel_size", 1.f / m_output_texture->width(), 0);
	t->bind(0);


	if (toFBO)
	{
		m_fbo->bind();
		Gcon.clear(BuffBit::COLOR);
		Gcon.setViewport(m_output_texture->width(), m_output_texture->height());
		Effect::renderDefaultVAO();
		m_fbo->unbind();
		Gcon.setViewport(APwin()->getWidth(), APwin()->getHeight());
	}
	else
		Effect::renderDefaultVAO();
}

VerticalBlur::VerticalBlur(const TextureInfo& targetTexture)
	: SingleTextureEffect(targetTexture)
{
}

void VerticalBlur::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	std::static_pointer_cast<internal::GLShader>(getShader())->setUniform2f(
		"u_pixel_size", 0, 1.f / m_output_texture->height());
	t->bind(0);


	if (toFBO)
	{
		m_fbo->bind();
		Gcon.clear(BuffBit::COLOR);
		Gcon.setViewport(m_output_texture->width(), m_output_texture->height());
		Effect::renderDefaultVAO();
		m_fbo->unbind();
	}
	else
		Effect::renderDefaultVAO();
}

GaussianBlur::GaussianBlur(const TextureInfo& targetTexture)
	: m_hor(targetTexture), m_vert(targetTexture)
{
}

void GaussianBlur::render(const Texture* t, bool toFBO)
{
	m_hor.render(t);
	m_vert.render(m_hor.getTexture(), toFBO);
}

GaussianBlurMultiple::GaussianBlurMultiple(const TextureInfo& targetTexture, std::initializer_list<float> scales)
{
	TextureInfo copy = targetTexture;
	copy.filterMode(TextureFilterMode::LINEAR);
	m_list.reserve(scales.size());
	for (float scale : scales)
	{
		m_scales.push_back(scale);
		copy.size(targetTexture.width / scale, targetTexture.height / scale);
		m_list.emplace_back(copy);
	}
}

void GaussianBlurMultiple::render(const Texture* t)
{
	auto tt = t;
	for (auto& i : m_list)
	{
		i.render(tt);
		tt = i.getTexture();
	}
}
}
