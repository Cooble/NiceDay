#include "ndpch.h"
#include "OpenGLFrameBuffer.h"
#include "OpenGLRenderer.h"

OpenGLFrameBuffer::OpenGLFrameBuffer()
{
	GLCall(glGenFramebuffers(1, &m_id));
}

OpenGLFrameBuffer::~OpenGLFrameBuffer()
{
	GLCall(glDeleteFramebuffers(1, &m_id));
}

void OpenGLFrameBuffer::bind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_id));
}

void OpenGLFrameBuffer::unbind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void OpenGLFrameBuffer::attachTexture(unsigned int textureId)
{
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));
}