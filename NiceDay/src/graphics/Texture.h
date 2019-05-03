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
	std::string m_filePath;

public:
	Texture(const std::string& file_path,GLenum filter_mode = GL_LINEAR, GLenum wrap_mode=GL_REPEAT);

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }

	~Texture();
};

