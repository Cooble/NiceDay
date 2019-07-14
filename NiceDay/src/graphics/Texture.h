#pragma once
#include "ndpch.h"
#include "glad/glad.h"

enum class TextureWrapMode
{
	REPEAT = GL_REPEAT,
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
};

enum class TextureFilterMode
{
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR
};

enum class TextureFormat
{
	RGBA = GL_RGBA,
	RGB = GL_RGB,
	RED = GL_RED
};

struct TextureInfo
{
	float border_color[4] = {0, 0, 0, 0}; //transparent color outside of image
	int width = 0, height = 0;
	std::string file_path;
	TextureFilterMode filter_mode_min = TextureFilterMode::LINEAR;
	TextureFilterMode filter_mode_max = TextureFilterMode::LINEAR;

	TextureWrapMode wrap_mode_s = TextureWrapMode::REPEAT;
	TextureWrapMode wrap_mode_t = TextureWrapMode::REPEAT;
	TextureFormat f_format = TextureFormat::RGBA;

	inline TextureInfo& filterMode(TextureFilterMode mode)
	{
		filter_mode_min = mode;
		filter_mode_max = mode;
		return *this;
	}

	inline TextureInfo& wrapMode(TextureWrapMode mode)
	{
		wrap_mode_s = mode;
		wrap_mode_t = mode;
		return *this;
	}

	inline TextureInfo& path(const std::string& s)
	{
		file_path = s;
		return *this;
	}

	inline TextureInfo& size(int w, int h)
	{
		width = w;
		height = h;
		return *this;
	}

	inline TextureInfo& format(TextureFormat form)
	{
		f_format = form;
		return *this;
	}

	inline TextureInfo& borderColor(float r, float g, float b, float a)
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
public:
	virtual ~Texture() = default;

	virtual void bind(unsigned int slot = 0) const = 0;
	virtual void unbind() const = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	virtual unsigned int getID() const = 0;


	virtual void setPixels(float* light_map) = 0; //todo add template anotation to enable more than jut floats
	virtual void setPixels(uint8_t* light_map) =0; //todo add template anotation to enable more than jut bytes
public:
	static Texture* create(TextureInfo&);
};
