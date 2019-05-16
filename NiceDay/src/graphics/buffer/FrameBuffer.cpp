#include "ndpch.h"
#include "FrameBuffer.h"
#include "graphics/Renderer.h"

FrameBuffer::FrameBuffer()
{
	Call(glGenFramebuffers(1, &m_id));
}

FrameBuffer::~FrameBuffer()
{
	Call(glDeleteFramebuffers(1, &m_id));
}

void FrameBuffer::bind()
{
	Call(glBindFramebuffer(GL_FRAMEBUFFER, m_id));
}

void FrameBuffer::unbind()
{
	Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::attachTexture(unsigned int textureId)
{
	Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));
}
