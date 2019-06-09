#pragma once
#include "graphics/buffer/FrameBuffer.h"

class OpenGLFrameBuffer :public FrameBuffer
{
private:
	unsigned int m_id;
public:
	OpenGLFrameBuffer();
	~OpenGLFrameBuffer();
	void bind() override;
	void unbind() override;
	void attachTexture(unsigned textureId) override;


	
};
