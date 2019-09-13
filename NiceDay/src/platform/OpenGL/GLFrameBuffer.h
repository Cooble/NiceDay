#pragma once
#include "graphics/API/FrameBuffer.h"

class GLFrameBuffer :public FrameBuffer
{
private:
	unsigned int m_id;
	std::vector<unsigned int> m_color_attachments;
public:
	GLFrameBuffer();
	~GLFrameBuffer();
	void bind() override;
	void unbind() override;
	void attachTexture(unsigned textureId, unsigned int attachmentNumber) override;


	
};
