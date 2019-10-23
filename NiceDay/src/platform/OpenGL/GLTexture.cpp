#include "ndpch.h"
#include "GLTexture.h"
#include "stb_image.h"
#include "GLRenderer.h"


GLTexture::GLTexture(const TextureInfo& info)
	:m_width(info.width),
	m_height(info.height),
	m_format(info.f_format),
	m_filePath(ND_RESLOC(info.file_path))
{
	GLCall(glGenTextures(1, &m_id));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_id));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)info.filter_mode_min));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)info.filter_mode_max));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)info.wrap_mode_s));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)info.wrap_mode_t));

	if (info.wrap_mode_s == TextureWrapMode::CLAMP_TO_BORDER || info.wrap_mode_t == TextureWrapMode::CLAMP_TO_BORDER)
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, info.border_color);

	if (!info.file_path.empty())
	{
		stbi_set_flip_vertically_on_load(true);
		m_buffer = stbi_load(m_filePath.c_str(), &m_width, &m_height, &m_BPP, 4);
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, (int)m_format, m_width, m_height, 0, (int)m_format, GL_UNSIGNED_BYTE, m_buffer));
		if (m_buffer)
			stbi_image_free(m_buffer);
		else
			ND_ERROR("Invalid texture path: {}", m_filePath.c_str());
	}
	else
	{
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, (int)m_format, m_width, m_height, 0, (int)m_format, GL_UNSIGNED_BYTE, nullptr));
	}
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}


GLTexture::~GLTexture()
{
	GLCall(glDeleteTextures(1, &m_id));
}

void GLTexture::setPixels(float* light_map)
{
	GLCall(glBindTexture(GL_TEXTURE_2D, m_id));
	GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, (int)m_format, GL_FLOAT, light_map));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
void GLTexture::setPixels(uint8_t* light_map)
{
	GLCall(glBindTexture(GL_TEXTURE_2D, m_id));
	GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, (int)m_format, GL_UNSIGNED_BYTE, light_map));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void GLTexture::bind(unsigned int slot) const {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_id));
}
void GLTexture::unbind() const {
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}