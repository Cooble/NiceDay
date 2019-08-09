﻿#include "ndpch.h"
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
	if(m_color_attachments.size())
		GLCall(glDrawBuffers(m_color_attachments.size(), m_color_attachments.data()));
}

void OpenGLFrameBuffer::unbind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void OpenGLFrameBuffer::attachTexture(unsigned int textureId, unsigned int attachmentNumber)
{
	attachmentNumber += GL_COLOR_ATTACHMENT0;

	if (textureId == 0) {//remove no-longer-used attachment port
		for (int i = 0; i < m_color_attachments.size(); ++i)
		{
			if (m_color_attachments[i] == attachmentNumber)
			{
				m_color_attachments.erase(m_color_attachments.begin() + i);
				break;
			}

		}
	}
	else {
		bool alreadyActive = false;
		for (unsigned int m_color_attachment : m_color_attachments)
			if (m_color_attachment == attachmentNumber)
			{
				alreadyActive = true;
				break;
			}
		if (!alreadyActive)
			 m_color_attachments.push_back(attachmentNumber);
	}
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentNumber, GL_TEXTURE_2D, textureId, 0));
}