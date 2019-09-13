#pragma once
#include "graphics/GContext.h"

class GLContext:public GContext
{
private:
	bool m_blend_enabled=false;
	struct BlendData
	{
		BlendEquation blendEquation;
		Blend blends[4];
		float constant[4];
	} m_current_blend_data;
	float m_clearColor[4];
	int m_viewport[4];
public:
	~GLContext() = default;
	void enableBlend() override;
	void disableBlend() override;
	void setBlendEquation(BlendEquation e) override;
	void setBlendFuncSeparate(Blend src, Blend dst, Blend srcA, Blend dstA) override;
	void setBlendFunc(Blend src, Blend dst) override;
	void setBlendConstant(float r, float g, float b, float a) override;
	void clear(BufferBit bits) override;
	void setClearColor(float r, float g, float b, float a) override;
	void setViewport(int x, int y, int width, int height) override;
};



