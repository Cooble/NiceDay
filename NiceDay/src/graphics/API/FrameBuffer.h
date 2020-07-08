#pragma once
#include "Texture.h"
#include "graphics/GContext.h"

// target is normal fbo or actual application window
enum class FBType :uint32_t
{
	// have its own internal texture attachments
	// resize method can be called
	NORMAL_TARGET=0,
	// represent only husk which external textures need to be attached to
	// since it does not own attachments,
	//					- resize method cannot be used
	//					- viewport needs to be called manually
	// NOTE: Attached textures are not deleted on destruction!
	NORMAL_TARGET_EXTERNAL_TEXTURES=1,
	// its target is application window
	WINDOW_TARGET=2,
};
enum class FBAttachment :uint32_t
{
	NONE=0,
	COLOR=1,
	DEPTH_STENCIL=2,
	DEPTH_STENCIL_ACCESSIBLE=3
};


struct FrameBufferInfo
{
	//type of fbo
	FBType type= FBType::NORMAL_TARGET;

	//color attachment specifications
	TextureInfo* textureInfos;
	uint32_t textureInfoCount=0;

	//special attachment like depth or stencil
	//FBAttachment::COLOR cannot be used
	FBAttachment specialAttachment=FBAttachment::NONE;

private:
	//just for syntactic sugar
	//is completely ignored by FrameBuffer
	//usually textureInfos points to this
	TextureInfo singleTextureInfo;
public:
	FrameBufferInfo(const TextureInfo& info) { defaultTarget(info); }
	// dimensions CAN be zero!
	// its possible to prepare fbo with unknown dimensions
	// you just need to call resize() (which will implicitly create attachments)
	FrameBufferInfo(uint32_t width, uint32_t height, TextureFormat format) { defaultTarget(width, height, format); }
	FrameBufferInfo() { externalTarget(); }

	FrameBufferInfo& special(FBAttachment special) { specialAttachment = special; return *this; }
	FrameBufferInfo& windowTarget() { type = FBType::WINDOW_TARGET; return *this; }
	FrameBufferInfo& externalTarget() { type = FBType::NORMAL_TARGET_EXTERNAL_TEXTURES; return *this; }
	FrameBufferInfo& defaultTarget(uint32_t width,uint32_t height, TextureFormat format)
	{
		type = FBType::NORMAL_TARGET;
		
		singleTextureInfo.size(width,height);
		singleTextureInfo.format(format);
		textureInfoCount = 1;
		textureInfos = &singleTextureInfo;
		return *this;
	}
	// dimensions CAN be zero!
	// its possible to prepare fbo with unknown dimensions
	// you just need to call resize() (which will implicitly create attachments)
	FrameBufferInfo& defaultTarget(const TextureInfo& info)
	{
		type = FBType::NORMAL_TARGET;

		singleTextureInfo = info;
		textureInfoCount = 1;
		textureInfos = &singleTextureInfo;
		return *this;
	}
};

class FrameBuffer
{
public:
	virtual ~FrameBuffer() = default;
	virtual void bind() = 0;
	virtual void unbind() = 0;
	virtual void attachTexture(uint32_t textureId, uint32_t attachmentNumber) = 0;
	virtual void resize(uint32_t width, uint32_t height)=0;
	virtual TexDimensions getSize() const = 0;
	virtual void clear(BufferBit bits, const glm::vec4& color={0,0,0,0}) = 0;

	static FrameBuffer* create(const FrameBufferInfo& info={});
	virtual bool isWindow() const = 0;
	virtual uint32_t getAttachmentID(uint32_t attachmentIndex,FBAttachment type=FBAttachment::COLOR) const = 0;
	virtual const Texture* getAttachment(uint32_t attachmentIndex=0, FBAttachment type = FBAttachment::COLOR) = 0;
};
