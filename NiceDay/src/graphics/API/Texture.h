#pragma once
#include "ndpch.h"
#include "glad/glad.h"

typedef glm::vec<2, uint32_t> TexDimensions;

enum class TextureWrapMode :uint32_t
{
	REPEAT = GL_REPEAT,
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
};

enum class TextureFilterMode :uint32_t
{
	NEAREST = GL_NEAREST,
	LINEAR = GL_LINEAR
};

enum class TextureFormat :uint32_t
{
	RGBA = GL_RGBA,
	RGB = GL_RGB,
	RED = GL_RED
};
enum class TextureType:uint32_t
{
	_2D = GL_TEXTURE_2D,
	_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
	_3D = GL_TEXTURE_3D,


};

//if texture type is cubemap ->
//		the path must be in form of '/blahblah/folder/some_prefix_name*.png'
//		the '*' character will be automatically replaced with px,nx,py,ny,pz,nz									
struct TextureInfo
{
	float border_color[4] = {0, 0, 0, 0}; //transparent color outside of image
	int width = 0, height = 0;
	std::string file_path;
	TextureFilterMode filter_mode_min = TextureFilterMode::LINEAR;
	TextureFilterMode filter_mode_max = TextureFilterMode::LINEAR;
	TextureType texture_type = TextureType::_2D;
	TextureWrapMode wrap_mode_s = TextureWrapMode::REPEAT;
	TextureWrapMode wrap_mode_t = TextureWrapMode::REPEAT;
	TextureWrapMode wrap_mode_r = TextureWrapMode::REPEAT;
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
		wrap_mode_r = mode;
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
	inline TextureInfo& size(int d)
	{
		width = d;
		height = d;
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
	inline TextureInfo& type(TextureType type)
	{
		texture_type = type;
	
		return *this;
	}

	inline TextureInfo copy() const { return *this; }

	TextureInfo()
	{
		file_path = "";
	}

	TextureInfo(const std::string& string)
	{
		file_path = string;
	}
	TextureInfo(TextureType type,const std::string& string)
	{
		texture_type = type;
		if (type == TextureType::_CUBE_MAP) {
			wrapMode(TextureWrapMode::CLAMP_TO_EDGE);
			format(TextureFormat::RGB);
		}
		file_path = string;
	}

	/*TextureInfo(const char* string)
	{
		file_path = string;
	}*/
};

class Texture
{
public:
	virtual ~Texture() = default;

	virtual void bind(unsigned int slot = 0) const = 0;
	virtual void unbind() const = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
	TexDimensions getDimensions() const { return { getWidth(),getHeight() }; }
	virtual unsigned int getID() const = 0;

	virtual const std::string& getFilePath() const = 0;
	virtual void setPixels(float* light_map) = 0; //todo add template anotation to enable more than jut floats
	virtual void setPixels(uint8_t* light_map) =0; //todo add template anotation to enable more than jut bytes
public:
	static Texture* create(const TextureInfo&);
	
};
typedef Ref<Texture> TexturePtr;

class TextureLib
{
private:
	inline static std::unordered_map<std::string, TexturePtr> m_textures;
public:
	// if neccessary, loads texture, otherwise retrives already loaded
	static TexturePtr loadOrGetTexture(const std::string& path,TextureType type=TextureType::_2D)
	{
		auto it = m_textures.find(path);
		if (it == m_textures.end()) {
			auto t = std::shared_ptr<Texture>(Texture::create(TextureInfo(type,path)));
			m_textures[path] = t;
			return t;
		}
		return it->second;
	}
	static void unloadAll()
	{
		m_textures.clear();
	}
};



