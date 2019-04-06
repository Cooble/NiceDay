#include "ndpch.h"
#include "Texture.h"
#include "Renderer.h"
#include <stb_image.h>



Texture::Texture(const std::string& filePath)
	:m_id(0), m_buffer(nullptr), m_width(0), m_height(0),
	m_filePath(filePath), m_BPP(0)
{
	stbi_set_flip_vertically_on_load(true);
	m_buffer = stbi_load(filePath.c_str(), &m_width, &m_height, &m_BPP, 4);

	Call(glGenTextures(1, &m_id));
	Call(glBindTexture(GL_TEXTURE_2D, m_id));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
	Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));

	Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer));

	Call(glBindTexture(GL_TEXTURE_2D, 0));

	if (m_buffer) {
		stbi_image_free(m_buffer);
	}

}

Texture::~Texture()
{
	Call(glDeleteTextures(1, &m_id));
}
void Texture::bind(unsigned int slot) const {
	Call(glActiveTexture(GL_TEXTURE0 + slot));
	Call(glBindTexture(GL_TEXTURE_2D, m_id));
}
void Texture::unbind() const {
	Call(glBindTexture(GL_TEXTURE_2D, 0));
}
