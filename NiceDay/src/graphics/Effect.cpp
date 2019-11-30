#include "ndpch.h"
#include "Effect.h"
#include "platform/OpenGL/GLRenderer.h"
#include "GContext.h"
#include "core/App.h"


void Effect::renderDefault()
{
	s_vao->bind();
	GLCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void Effect::renderToScreen(Texture* t)
{
	s_shader->bind();
	t->bind(0);
	renderDefault();
}

FrameBufferTexturePair::FrameBufferTexturePair(Texture* t)
	:m_texture(nullptr)
{
	m_fbo = FrameBuffer::create();
	replaceTexture(t);
}

FrameBufferTexturePair::FrameBufferTexturePair():m_texture(nullptr)
{
	m_fbo = FrameBuffer::create();
}

FrameBufferTexturePair::~FrameBufferTexturePair()
{
	delete m_fbo;
	if(m_texture)
		delete m_texture;
}

void FrameBufferTexturePair::bind() const
{
	ASSERT(m_texture, "Cannot bind fbo, without texture attachment!");
	m_fbo->bind();
	Gcon.setViewport(m_texture->getWidth(), m_texture->getHeight());
}

void FrameBufferTexturePair::unbind() const
{
	m_fbo->unbind();
	Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
}

SingleTextureEffect::SingleTextureEffect(const TextureInfo& targetTexture)
{
	m_output_texture = Texture::create(targetTexture);
	m_fbo = FrameBuffer::create();
	m_fbo->bind();
	m_fbo->attachTexture(m_output_texture->getID(), 0);
	m_fbo->unbind();
}

SingleTextureEffect::~SingleTextureEffect()
{
	delete m_output_texture;
	delete m_fbo;
}

AlphaMaskEffect::AlphaMaskEffect(const TextureInfo& targetTexture)
	:SingleTextureEffect(targetTexture.copy().format(TextureFormat::RED))
{
}

void AlphaMaskEffect::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	t->bind(0);

	if (toFBO) {
		m_fbo->bind();
		Gcon.clear(COLOR_BUFFER_BIT);
		Gcon.setViewport(m_output_texture->getWidth(), m_output_texture->getHeight());
		Effect::renderDefault();
		m_fbo->unbind();
		Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	}
	else
		Effect::renderDefault();
}

ScaleEdgesEffect::ScaleEdgesEffect(const TextureInfo& targetTexture)
	:SingleTextureEffect(targetTexture)
{
}

void ScaleEdgesEffect::render(const Texture* t, float scale, bool toFBO)
{
	getShader()->bind();
	dynamic_cast<GLShader*>(getShader())->setUniform1f("u_scale", scale);
	t->bind(0);

	if (toFBO) {
		m_fbo->bind();
		Gcon.clear(COLOR_BUFFER_BIT);
		Gcon.disableBlend();
		Gcon.setViewport(m_output_texture->getWidth(), m_output_texture->getHeight());
		Effect::renderDefault();
		m_fbo->unbind();
		Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	}
	else
		Effect::renderDefault();
	getShader()->unbind();


}

GreenFilter::GreenFilter(const TextureInfo& targetTexture)
	:SingleTextureEffect(targetTexture)
{	
}

void GreenFilter::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	t->bind(0);

	if (toFBO) {
		m_fbo->bind();
		Gcon.setViewport(m_output_texture->getWidth(), m_output_texture->getHeight());
		Effect::renderDefault();
		m_fbo->unbind();
		Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	}
	else
		Effect::renderDefault();
	


}

HorizontalBlur::HorizontalBlur(const TextureInfo& targetTexture)
	:SingleTextureEffect(targetTexture)
{
}

void HorizontalBlur::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	dynamic_cast<GLShader*>(getShader())->setUniform2f("u_pixel_size", 1.f / m_output_texture->getWidth(), 0);
	t->bind(0);


	if (toFBO) {
		m_fbo->bind();
		Gcon.clear(COLOR_BUFFER_BIT);
		Gcon.setViewport(m_output_texture->getWidth(), m_output_texture->getHeight());
		Effect::renderDefault();
		m_fbo->unbind();
		Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	}
	else
		Effect::renderDefault();

}

VerticalBlur::VerticalBlur(const TextureInfo& targetTexture)
	:SingleTextureEffect(targetTexture)
{
}

void VerticalBlur::render(const Texture* t, bool toFBO)
{
	getShader()->bind();
	dynamic_cast<GLShader*>(getShader())->setUniform2f("u_pixel_size",0, 1.f / m_output_texture->getHeight());
	t->bind(0);


	if (toFBO) {
		m_fbo->bind();
		Gcon.clear(COLOR_BUFFER_BIT);
		Gcon.setViewport(m_output_texture->getWidth(), m_output_texture->getHeight());
		Effect::renderDefault();
		m_fbo->unbind();
		Gcon.setViewport(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());

	}
	else
		Effect::renderDefault();
}

GaussianBlur::GaussianBlur(const TextureInfo& targetTexture)
:m_hor(targetTexture),m_vert(targetTexture){
	
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
