#include "ndpch.h"
#include "Texture.h"
#include "Renderer.h"
#include <stb_image.h>


Texture::Texture(const TextureInfo& info)
	:m_width(info.width),
	m_height(info.height),
	m_format(info.f_format),
	m_filePath(info.file_path)
{
	Call(glGenTextures(1, &m_id));
	Call(glBindTexture(GL_TEXTURE_2D, m_id));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filter_mode_min));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filter_mode_max));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrap_mode_s));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrap_mode_t));

	if(info.wrap_mode_s== GL_CLAMP_TO_BORDER|| info.wrap_mode_t == GL_CLAMP_TO_BORDER)
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, info.border_color);

	if (!info.file_path.empty())
	{
		stbi_set_flip_vertically_on_load(true);
		m_buffer = stbi_load(info.file_path.c_str(), &m_width, &m_height, &m_BPP, 4);
		Call(glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_width, m_height, 0, m_format, GL_UNSIGNED_BYTE, m_buffer));
		if (m_buffer)
			stbi_image_free(m_buffer);

	}
	else
	{
		Call(glTexImage2D(GL_TEXTURE_2D, 0, m_format, m_width, m_height, 0, m_format, GL_UNSIGNED_BYTE, nullptr));
	}
	Call(glBindTexture(GL_TEXTURE_2D, 0));
}


Texture::~Texture()
{
	Call(glDeleteTextures(1, &m_id));
}

void Texture::setPixels(float* light_map)
{
	Call(glBindTexture(GL_TEXTURE_2D, m_id));
	Call(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_format, GL_FLOAT, light_map));
	Call(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::bind(unsigned int slot) const {
	Call(glActiveTexture(GL_TEXTURE0 + slot));
	Call(glBindTexture(GL_TEXTURE_2D, m_id));
}
void Texture::unbind() const {
	Call(glBindTexture(GL_TEXTURE_2D, 0));
}
