#pragma once
#include "graphics/API/Texture.h"


class GLTexture: public Texture
{
private:
	bool m_not_proxy = true;
	unsigned int m_id;
	int m_BPP;//channels
	unsigned char* m_buffer;
	int m_width, m_height;
	TextureType m_type;
	TextureFormat m_format;//should i use GLenum instead?
	std::string m_filePath;
	TextureInfo m_info;

public:
	GLTexture(const TextureInfo& info);
	~GLTexture();

	//will just wrap around existing texture, will not destroy it in destructor
	GLTexture(uint32_t id, uint32_t width, uint32_t height, TextureType type);
	GLTexture(uint32_t id, const TextureInfo& info);

	void bind(unsigned int slot = 0) const override;
	void unbind() const override;

	inline int width() const override { return m_width; }
	inline int height() const override { return m_height; }
	inline unsigned int getID() const override { return m_id; }

	const TextureInfo& getInfo() const { return m_info; }
	void setPixels(float* pixels) override;//todo add template anotation to enable more than jut floats
	void setPixels(uint8_t* pixels) override;//todo add template anotation to enable more than jut floats
	bool isProxy() const { return !m_not_proxy; }

	const std::string& getFilePath() const override { return m_info.file_path; }

};

