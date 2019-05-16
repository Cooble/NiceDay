#pragma once

class FrameBuffer
{
private:
	unsigned int m_id;

public:
	FrameBuffer();
	~FrameBuffer();
	void bind();
	void unbind();
	void attachTexture(unsigned int textureId);
};
