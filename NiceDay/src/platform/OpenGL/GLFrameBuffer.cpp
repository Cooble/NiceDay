#include "ndpch.h"
#include "GLFrameBuffer.h"
#include "GLRenderer.h"
#include "GLTexture.h"
#include "core/App.h"

//todo add possibility to add depth buffer when using external textures

void GLFrameBuffer::bindColorAttachments()
{
	unsigned int id_offset = 0;
	for (auto& attachment : m_attachments)
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (id_offset++), GL_TEXTURE_2D, attachment.texture->getID(), 0));

	//specifies specific color attachments to be drawn into
	constexpr int maxAttachments = 16;
	ASSERT(m_attachments.size()<=maxAttachments, "Too many attachments");
	GLenum types_atta[maxAttachments];
	for (int i = 0; i < m_attachments.size(); ++i)
		types_atta[i] = GL_COLOR_ATTACHMENT0 + i;
	GLCall(glDrawBuffers(m_attachments.size(), types_atta));
}

void GLFrameBuffer::resizeAttachments()
{
	for (int i = 0; i < m_attachments.size(); ++i)
	{
		auto& a = m_attachments[i];
		auto info = a.texture->getInfo();
		info.size(m_dimensions.x, m_dimensions.y);
		delete a.texture;
		a.texture = (GLTexture*)Texture::create(info);
	}
	bindColorAttachments();

	if (m_special_attachment_id != 0) {
		switch (m_special_attachment)
		{
		case FBAttachment::DEPTH_STENCIL:
			glDeleteRenderbuffers(1, &m_special_attachment_id);
			break;
		case FBAttachment::DEPTH_STENCIL_ACCESSIBLE:
			glDeleteTextures(1, &m_special_attachment_id);
			break;
		case FBAttachment::NONE: break;
		}
	}
	createBindSpecialAttachment();
}


void GLFrameBuffer::createBindSpecialAttachment()
{
	switch (m_special_attachment)
	{
	case FBAttachment::DEPTH_STENCIL:
		glGenRenderbuffers(1, &m_special_attachment_id);
		glBindRenderbuffer(GL_RENDERBUFFER, m_special_attachment_id);
		if(multiSampleLevel>1)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, multiSampleLevel, GL_DEPTH24_STENCIL8, m_dimensions.x, m_dimensions.y);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_dimensions.x, m_dimensions.y);

		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
		                          m_special_attachment_id));
		break;
	case FBAttachment::DEPTH_STENCIL_ACCESSIBLE:
		glGenTextures(1, &m_special_attachment_id);
		glBindTexture(GL_TEXTURE_2D, m_special_attachment_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_dimensions.x, m_dimensions.y, 0,
		             GL_DEPTH_STENCIL,
		             GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_special_attachment_id, 0));
		break;
	case FBAttachment::NONE: break;
	default: ;
	}
}


void GLFrameBuffer::clearAttachments()
{
	for (auto& attachment : m_attachments) {
		delete attachment.texture;
	}
	if (m_special_attachment_id != 0) {
		switch (m_special_attachment)
		{
		case FBAttachment::DEPTH_STENCIL:
			glDeleteRenderbuffers(1, &m_special_attachment_id);
			break;
		case FBAttachment::DEPTH_STENCIL_ACCESSIBLE:
			glDeleteTextures(1, &m_special_attachment_id);
			break;
		case FBAttachment::NONE: break;
		}
	}
}

GLFrameBuffer::GLFrameBuffer(const FrameBufferInfo& info):m_special_attachment(FBAttachment::NONE)
{
	m_id = 0;
	m_type = info.type;
	
	if (m_type == FBType::WINDOW_TARGET)
		return;
	
	GLCall(glGenFramebuffers(1, &m_id));
	if (m_type == FBType::NORMAL_TARGET) {
		ASSERT(info.textureInfoCount, "At least one color attachment is neccessary (for some reason idk, Because I said so..)");
		m_dimensions.x = info.textureInfos[0].width;
		m_dimensions.y = info.textureInfos[0].height;
		//ASSERT((bool)m_dimensions.x && (bool)m_dimensions.y, "Invalid fbo dimensions");
		m_attachments.resize(info.textureInfoCount);
		m_special_attachment = info.specialAttachment;

		bool noDimSpecified = false;
		for (int i = 0; i < m_attachments.size(); ++i)
		{
			auto& a = m_attachments[i];
			if(info.textureInfos[i].width==0||info.textureInfos[i].height==0)
			{
				//we have made invalid proxy
				a.texture = new GLTexture(0,info.textureInfos[i]);
				noDimSpecified = true;
			}else
			{
				a.texture = new GLTexture(info.textureInfos[i]);
			}
		}
		if (!noDimSpecified) {
			bind();
			bindColorAttachments();
			createBindSpecialAttachment();
		}
	}

}


GLFrameBuffer::~GLFrameBuffer()
{
	if (m_type == FBType::WINDOW_TARGET)
		return;
	GLCall(glDeleteFramebuffers(1, &m_id));
	clearAttachments();
}

void GLFrameBuffer::createBindSpecialAttachment(FBAttachment attachment, TexDimensions dim)
{
	if (m_type == FBType::NORMAL_TARGET_EXTERNAL_TEXTURES) {
		if (m_special_attachment== FBAttachment::NONE)
		{
			m_dimensions = dim;
			m_special_attachment = attachment;
			bind();
			createBindSpecialAttachment();
		}
	}
}

void GLFrameBuffer::bind()
{
	if (s_currently_bound == this)
		return;

	s_currently_bound = this;

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_id));
	if (isWindow())
	{
		auto dim = App::get().getPhysicalWindow()->getDimensions();
		glViewport(0, 0, dim.x, dim.y);
	}
	else if (m_type == FBType::NORMAL_TARGET)
	{
		glViewport(0, 0, m_dimensions.x, m_dimensions.y);
	}
	else
	{
		//viewport cannot be established since size is not known
	}

}

void GLFrameBuffer::unbind()
{
	Renderer::getDefaultFBO()->bind();
}

void GLFrameBuffer::resize(uint32_t width, uint32_t height)
{
	if (width == m_dimensions.x && height == m_dimensions.y)
		return;
	m_dimensions = {width, height};
	bind();
	resizeAttachments();
}

TexDimensions GLFrameBuffer::getSize() const
{
	ASSERT(m_type == FBType::NORMAL_TARGET, "");
	return m_dimensions;
}

void GLFrameBuffer::clear(BufferBit bits, const glm::vec4& color)
{
	bind();
	//ASSERT(m_is_bound, "Cannot clear fbo that is not bound");
	if (bits & BuffBit::COLOR)
		glClearColor(color.r, color.g, color.b, color.a);
	//else if (bits & BuffBit::DEPTH_BUFFER_BIT)
	//	glClearDepth(color.r);
	glClear(bits);
}

void GLFrameBuffer::attachTexture(uint32_t textureId, uint32_t attachmentNumber)
{
	ASSERT(m_type != FBType::WINDOW_TARGET, "Cannot attach texture to window target");
	bind();
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, GL_TEXTURE_2D, textureId, 0));
}

void GLFrameBuffer::attachTexture(Texture* t, uint32_t attachmentNumber)
{
	ASSERT(m_type != FBType::WINDOW_TARGET, "Cannot attach texture to window target");
	bind();
	glViewport(0, 0, t->getWidth(), t->getHeight());
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentNumber, GL_TEXTURE_2D, t->getID(), 0));
}

uint32_t GLFrameBuffer::getAttachmentID(uint32_t attachmentIndex, FBAttachment type) const
{
	ASSERT(m_type == FBType::NORMAL_TARGET, "");
	ASSERT(attachmentIndex < m_attachments.size(), "INVALID fbo attachment id");
	return m_attachments[attachmentIndex].texture->getID();
}

const Texture* GLFrameBuffer::getAttachment(uint32_t attachmentIndex, FBAttachment type)
{
	switch (type) { 
	case FBAttachment::COLOR:
	{
		if (attachmentIndex >= m_attachments.size())
			return nullptr;
		//ASSERT(attachmentIndex < m_attachments.size(),"Invalid attachment index");
		return m_attachments[attachmentIndex].texture;
	}
	case FBAttachment::DEPTH_STENCIL_ACCESSIBLE:
		//todo add depth stencil attachment texture retsrieveal
		
	default:
		ASSERT(false, "Invalid textureattachment request");
		return nullptr;
	}
}
