#pragma once
#include "ndpch.h"
#include "glad/glad.h"

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
	Texture(const std::string& file_path,GLenum filter_mode = GL_LINEAR, GLenum wrap_mode=GL_REPEAT);
	Texture(int width,int height,GLenum filter_mode = GL_LINEAR, GLenum wrap_mode=GL_REPEAT,GLenum format=GL_RGBA);

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }
	inline unsigned int getID() const { return m_id; }

	~Texture();
	
	void setPixels(float* light_map);//todo add template anotation to enable more than jut floats
};

