#pragma once
#include "graphics/buffer/FrameBuffer.h"

class OpenGLFrameBuffer :public FrameBuffer
{
private:
	unsigned int m_id;
	std::vector<unsigned int> m_color_attachments;
public:
	OpenGLFrameBuffer();
	~OpenGLFrameBuffer();
	void bind() override;
	void unbind() override;
	void attachTexture(unsigned textureId, unsigned int attachmentNumber) override;


	
};
