#include "ndpch.h"
#include "GLTexture.h"
#include "stb_image.h"
#include "GLRenderer.h"

static std::string replace(const std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return str;
	std::string out=str;
	out.replace(start_pos, from.length(), to);
	return out;
}

GLTexture::GLTexture(const TextureInfo& info)
	:m_width(info.width),
	m_height(info.height),
	m_format(info.f_format),
	m_filePath(ND_RESLOC(info.file_path)),
	m_info(info),
	m_type(info.texture_type)
{
	auto type = (uint32_t)info.texture_type;
	GLCall(glGenTextures(1, &m_id));
	
	GLCall(glBindTexture(type, m_id));
	GLCall(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, (int)info.filter_mode_min));
	GLCall(glTexParameteri(type, GL_TEXTURE_MAG_FILTER, (int)info.filter_mode_max));
	GLCall(glTexParameteri(type, GL_TEXTURE_WRAP_S, (int)info.wrap_mode_s));
	GLCall(glTexParameteri(type, GL_TEXTURE_WRAP_T, (int)info.wrap_mode_t));
	if(info.texture_type==TextureType::_CUBE_MAP)
		GLCall(glTexParameteri(type, GL_TEXTURE_WRAP_R, (int)info.wrap_mode_r));


	if (info.wrap_mode_s == TextureWrapMode::CLAMP_TO_BORDER || info.wrap_mode_t == TextureWrapMode::CLAMP_TO_BORDER)
		GLCall(glTexParameterfv(type, GL_TEXTURE_BORDER_COLOR, info.border_color));

	if (!info.file_path.empty())
	{
		std::string facesNames = "pxnxpynypznz";
		
		stbi_set_flip_vertically_on_load(true);
		if (info.texture_type == TextureType::_2D) {
			m_buffer = stbi_load(ND_RESLOC(m_filePath).c_str(), &m_width, &m_height, &m_BPP, 4);
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, (int)m_format, m_width, m_height, 0, (int)m_format, GL_UNSIGNED_BYTE, m_buffer));
			if (m_buffer)
				stbi_image_free(m_buffer);
			else {
				ND_ERROR("Invalid texture path: {}", m_filePath.c_str());
			}
		}
		else if(info.texture_type==TextureType::_CUBE_MAP)
		{
			stbi_set_flip_vertically_on_load(false);

			for (GLuint i = 0; i < 6; i++)
			{
				std::string realPath = replace(ND_RESLOC(m_filePath), "*", facesNames.substr(i * 2, 2));
				m_buffer = stbi_load(realPath.c_str(), &m_width, &m_height, &m_BPP, 3);
				GLCall(glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, (int)m_format, m_width, m_height, 0, (int)m_format, GL_UNSIGNED_BYTE, m_buffer));
				if (m_buffer)
					stbi_image_free(m_buffer);
				else
				{
					ND_ERROR("Invalid texture path: {}", realPath.c_str());
				}
			}
		}
	}
	else
	{
		// resolves problem with weird alignment of pixels
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		auto fixer = m_format == TextureFormat::RGB ? GL_RGB8 : (int)m_format;
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, fixer, m_width, m_height, 0, (int)m_format, GL_UNSIGNED_BYTE, nullptr));
	}
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}


GLTexture::~GLTexture()
{
	if(m_not_proxy)
		GLCall(glDeleteTextures(1, &m_id));
}

GLTexture::GLTexture(uint32_t id, uint32_t width, uint32_t height, TextureType type)
	:m_width(width),m_height(height),m_id(id),m_not_proxy(false),m_type(type)
{
}

GLTexture::GLTexture(uint32_t id, const TextureInfo& info)
	: m_width(info.width),
	m_height(info.height),
	m_format(info.f_format),
	m_id(id),
	m_filePath(ND_RESLOC(info.file_path)),
	m_info(info),
	m_type(info.texture_type),
	m_not_proxy(false)
{

}


void GLTexture::setPixels(float* light_map)
{
	ASSERT(m_not_proxy, "This texture is only proxy");
	GLCall(glBindTexture((int)m_type, m_id));
	GLCall(glTexSubImage2D((int)m_type, 0, 0, 0, m_width, m_height, (int)m_format, GL_FLOAT, light_map));
	GLCall(glBindTexture((int)m_type, 0));
}
void GLTexture::setPixels(uint8_t* light_map)
{
	ASSERT(m_not_proxy, "This texture is only proxy");
	GLCall(glBindTexture((int)m_type, m_id));
	GLCall(glTexSubImage2D((int)m_type, 0, 0, 0, m_width, m_height, (int)m_format, GL_UNSIGNED_BYTE, light_map));
	GLCall(glBindTexture((int)m_type, 0));
}

void GLTexture::bind(unsigned int slot) const {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture((int)m_type, m_id));
}
void GLTexture::unbind() const {
	GLCall(glBindTexture((int)m_type, 0));
}