#pragma once
#include "API/FrameBuffer.h"
#include "API/Texture.h"
#include "API/Buffer.h"
#include "API/VertexArray.h"
#include "API/Shader.h"
#include "platform/OpenGL/GLShader.h"

class Effect
{
private:
	inline static VertexBuffer* s_vbo = nullptr;
	inline static VertexArray* s_vao = nullptr;
	inline static Shader* s_shader = nullptr;

public:
	inline static void init()
	{
		if (s_vbo) //dont init again
			return;

		float f[]
		{
			//   x,y   u,v
			1, -1, 1, 0,
			-1, -1, 0, 0,
			1, 1, 1, 1,
			-1, 1, 0, 1,
		};
		s_vbo = VertexBuffer::create(f, sizeof(f));
		VertexBufferLayout layout;
		layout.push<float>(2); //pos
		layout.push<float>(2); //uv
		s_vbo->setLayout(layout);
		s_vao = VertexArray::create();
		s_vao->addBuffer(*s_vbo);

		s_shader = ShaderLib::loadOrGetShader("res/shaders/TextureQuad.shader");
	}

	//inline static const VertexBuffer& getDefaultVBO() { return *s_vbo; }

	// simple fullscreen quad with simple uv (location=0)=pos, (location=1)=uv,
	inline static const VertexArray& getDefaultVAO() { return *s_vao; }
	inline static const Shader& getDefaultShader() { return *s_shader; }

	// draws default vao (shader, textures etc has to be bound at this point)
	static void renderDefault();

	// draws texture on whole screen
	static void renderToCurrentFBO(Texture* t);
};

class FrameBufferTexturePair
{
	FrameBuffer* m_fbo;
	Texture* m_texture;

public:
	FrameBufferTexturePair(Texture* t);
	FrameBufferTexturePair();
	~FrameBufferTexturePair();

	inline void replaceTexture(Texture* t)
	{
		if (m_texture)
			delete m_texture;
		m_texture = t;

		m_fbo->bind();
		m_fbo->attachTexture(m_texture->getID(),0);
		m_fbo->unbind();
	}

	inline FrameBuffer* getFBO() { return m_fbo; }
	void bind() const;
	void unbind() const;
	inline Texture* getTexture() { return m_texture; }
};
class SingleTextureEffect
{
protected:
	FrameBuffer* m_fbo;
	Texture* m_output_texture;
public:
	SingleTextureEffect(const TextureInfo& targetTexture);
	~SingleTextureEffect();

	inline Texture* getTexture() { return m_output_texture; }

	inline void replaceTexture(const TextureInfo& targetTexture)
	{
		if(m_output_texture)
			delete m_output_texture;
		m_output_texture = Texture::create(targetTexture);
		m_fbo->bind();
		m_fbo->attachTexture(m_output_texture->getID(), 0);
		m_fbo->unbind();
	}
};
class AlphaMaskEffect:public SingleTextureEffect
{
private:
	Shader* getShader()
	{
		static Shader* s = nullptr;
		if (s == nullptr)
		{
			s = ShaderLib::loadOrGetShader("res/shaders/AlphaMask.shader");
			s->bind();
			dynamic_cast<GLShader*>(s)->setUniform1i("u_attachment", 0); //txture input
			s->unbind();
		}
		return s;
		return nullptr;
	}

public:
	AlphaMaskEffect(const TextureInfo& targetTexture);
	~AlphaMaskEffect() = default;

	void render(const Texture* t, bool toFBO = true);
};
class ScaleEdgesEffect:public SingleTextureEffect
{
private:
	Shader* getShader()
	{
		static Shader* s = nullptr;
		if (s == nullptr)
		{
			s = ShaderLib::loadOrGetShader("res/shaders/ScaleEdge.shader");
			s->bind();
			dynamic_cast<GLShader*>(s)->setUniform1i("u_attachment", 0); //txture input
			s->unbind();
		}
		return s;
		return nullptr;
	}

public:
	ScaleEdgesEffect(const TextureInfo& targetTexture);
	~ScaleEdgesEffect() = default;

	void render(const Texture* t,float scale, bool toFBO = true);
};

class GreenFilter :public SingleTextureEffect
{
private:
	Shader* getShader()
	{
		/*static Shader* s = nullptr;
		if (s == nullptr)
		{
			s = new Shader("res/shaders/GreenFilter.shader");
			s->bind();
			s->setUniform1i("u_attachment", 0); //txture input
			s->unbind();
		}
		return s;*/
		return nullptr;
	}

public:
	GreenFilter(const TextureInfo& targetTexture);
	~GreenFilter() = default;

	void render(const Texture* t, bool toFBO = true);
};

class GaussBlurShader
{
protected:
	Shader* getShader()
	{
		static Shader* s = nullptr;
		if (s == nullptr)
		{
			s = ShaderLib::loadOrGetShader("res/shaders/Blur.shader");
			s->bind();
			dynamic_cast<GLShader*>(s)->setUniform1i("u_attachment", 0); //txture input
			s->unbind();
		}
		return s;
	}
};

class HorizontalBlur : GaussBlurShader,public SingleTextureEffect
{

public:
	HorizontalBlur(const TextureInfo& targetTexture);
	~HorizontalBlur() = default;

	void render(const Texture* t, bool toFBO = true);
};
class VerticalBlur : GaussBlurShader, public SingleTextureEffect
{

public:
	VerticalBlur(const TextureInfo& targetTexture);
	~VerticalBlur() = default;

	void render(const Texture* t, bool toFBO = true);
};
class GaussianBlur
{
	HorizontalBlur m_hor;
	VerticalBlur m_vert;
public:

	GaussianBlur(const TextureInfo& targetTexture);
	void render(const Texture* t, bool toFBO = true);
	inline Texture* getTexture() { return m_vert.getTexture(); }
	
	inline void replaceTexture(const TextureInfo& targetTexture)
	{
		m_hor.replaceTexture(targetTexture);
		m_vert.replaceTexture(targetTexture);
	}


};
class GaussianBlurMultiple
{
	std::vector<GaussianBlur> m_list;
	std::vector<float> m_scales;
public:

	GaussianBlurMultiple(const TextureInfo& targetTexture, std::initializer_list<float> scales);
	
	void render(const Texture* t);

	inline void replaceTexture(const TextureInfo& targetTexture)
	{
		TextureInfo copy = targetTexture;

		for (int i = 0; i < m_list.size(); ++i)
		{
			float scale = m_scales[i];
			copy.size(targetTexture.width / scale, targetTexture.height / scale);
			m_list[i].replaceTexture(copy);
		}
	}

	inline Texture* getTexture() 
	{
		return  m_list[m_list.size() - 1].getTexture();
	}

};
