#pragma once
#include "graphics/API/Texture.h"


class GLTexture: public Texture
{
private:
	unsigned int m_id;
	int m_BPP;//channels
	unsigned char* m_buffer;
	int m_width, m_height;
	TextureFormat m_format;//should i use GLenum instead?
	std::string m_filePath;

public:
	GLTexture(const TextureInfo& info);
	~GLTexture();

	void bind(unsigned int slot = 0) const override;
	void unbind() const override;

	inline int getWidth() const override { return m_width; }
	inline int getHeight() const override { return m_height; }
	inline unsigned int getID() const override { return m_id; }


	void setPixels(float* light_map) override;//todo add template anotation to enable more than jut floats
	void setPixels(uint8_t* light_map) override;//todo add template anotation to enable more than jut floats
};

