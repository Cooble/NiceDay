#pragma once
#include <utility>
#include "ndpch.h"
#include "glad/glad.h"

struct TextureInfo
{
	float border_color[4] = {0,0,0,0};//transparent color outside of image
	int width=0, height=0;
	std::string file_path;
	GLenum filter_mode_min=GL_LINEAR;
	GLenum filter_mode_max= GL_LINEAR;
	GLenum wrap_mode_s=GL_REPEAT;
	GLenum wrap_mode_t= GL_REPEAT;
	GLenum f_format = GL_RGBA;

	TextureInfo& filterMode(GLenum mode)
	{
		filter_mode_min = mode;
		filter_mode_max= mode;
		return *this;
	}
	TextureInfo& wrapMode(GLenum mode)
	{
		wrap_mode_s = mode;
		wrap_mode_t = mode;
		return *this;
	}
	TextureInfo& path(const std::string& s)
	{
		file_path = s;
		return *this;
	}

	TextureInfo& size(int w, int h)
	{
		width = w;
		height = h;
		return *this;
	}
	TextureInfo& format(GLenum form)
	{
		f_format = form;
		return *this;
	}
	TextureInfo& borderColor(float r,float g,float b,float a)
	{
		border_color[0] = r;
		border_color[1] = g;
		border_color[2] = b;
		border_color[3] = a;
		return *this;
	}


	TextureInfo()
	{
		file_path = "";
	}
	TextureInfo(const std::string& string)
	{
		file_path = string;
	}
	TextureInfo(const char* string)
	{
		file_path = string;
	}

};
class Texture
{
private:
	unsigned int m_id;
	int m_BPP;//channels
	unsigned char* m_buffer;
	int m_width, m_height;
	GLenum m_format;
	std::string m_filePath;

public:
	Texture(const TextureInfo& info);

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }
	inline unsigned int getID() const { return m_id; }

	~Texture();
	
	void setPixels(float* light_map);//todo add template anotation to enable more than jut floats
	void setPixels(uint8_t* light_map);//todo add template anotation to enable more than jut floats
};

