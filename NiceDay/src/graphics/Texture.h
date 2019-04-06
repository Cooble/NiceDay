#pragma once
#include "ndpch.h"

class Texture
{
private:
	unsigned int m_id;
	int m_BPP;//channels
	unsigned char* m_buffer;
	int m_width, m_height;
	std::string m_filePath;

public:
	Texture(const std::string& filePath);

	void bind(unsigned int slot = 0) const;
	void unbind() const;

	inline int getWidth() const { return m_width; }
	inline int getHeight() const { return m_height; }

	~Texture();
};

