#include "ndpch.h"
#include "OpenGLFrameBuffer.h"
#include <glad/glad.h>
#include "graphics/Renderer.h"

OpenGLFrameBuffer::OpenGLFrameBuffer()
{
	Call(glGenFramebuffers(1, &m_id));
}

OpenGLFrameBuffer::~OpenGLFrameBuffer()
{
	Call(glDeleteFramebuffers(1, &m_id));
}

void OpenGLFrameBuffer::bind()
{
	Call(glBindFramebuffer(GL_FRAMEBUFFER, m_id));
}

void OpenGLFrameBuffer::unbind()
{
	Call(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void OpenGLFrameBuffer::attachTexture(unsigned int textureId)
{
	Call(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));
}