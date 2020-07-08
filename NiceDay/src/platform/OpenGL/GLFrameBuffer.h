#pragma once
#include "graphics/API/FrameBuffer.h"

class GLTexture;
class GLFrameBuffer :public FrameBuffer
{
private:
	struct ColorAttachment { GLTexture* texture=nullptr; };
	
	inline static GLFrameBuffer* s_currently_bound=nullptr;
	
	unsigned int m_id;
	FBType m_type;
	
	std::vector<ColorAttachment> m_attachments;
	FBAttachment m_special_attachment;
	uint32_t m_special_attachment_id;
	TexDimensions m_dimensions;

	void bindColorAttachments();
	void resizeAttachments();
	void createBindSpecialAttachment();
	void clearAttachments();
public:
	bool isWindow() const override { return m_type == FBType::WINDOW_TARGET; }
	GLFrameBuffer(const FrameBufferInfo& info);
	~GLFrameBuffer();
	void bind() override;
	void unbind() override;
	void resize(uint32_t width, uint32_t height) override;
	TexDimensions getSize() const override;

	void clear(BufferBit bits, const glm::vec4& color) override;
	void attachTexture(uint32_t textureId, uint32_t attachmentNumber) override;
	uint32_t getAttachmentID(uint32_t attachmentIndex, FBAttachment type = FBAttachment::COLOR) const override;
	const Texture* getAttachment(uint32_t attachmentIndex, FBAttachment type) override;
};

