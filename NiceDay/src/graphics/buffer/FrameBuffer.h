#pragma once

class FrameBuffer
{
public:
	virtual ~FrameBuffer() = default;
	virtual void bind() = 0;
	virtual void unbind() = 0;
	virtual void attachTexture(unsigned int textureId) = 0;

	static FrameBuffer* create();
};
