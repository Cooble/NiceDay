#pragma once
#include "graphics/GContext.h"
#include <glad/glad.h>

inline uint32_t toGL(g_typ type)
{
	switch(type)
	{
	case g_typ::UNSIGNED_INT:	return GL_UNSIGNED_INT;
	case g_typ::FLOAT:			return GL_FLOAT;
	case g_typ::INT:			return GL_INT;
	case g_typ::VEC2:			return GL_FLOAT_VEC2;
	case g_typ::VEC3:			return GL_FLOAT_VEC3;
	case g_typ::VEC4:			return GL_FLOAT_VEC4;
	case g_typ::MAT3:			return GL_FLOAT_MAT3;
	case g_typ::MAT4:			return GL_FLOAT_MAT4;

	case g_typ::IVEC2:			return GL_INT_VEC2;
	case g_typ::IVEC3:			return GL_INT_VEC3;
	case g_typ::IVEC4:			return GL_INT_VEC4;
	case g_typ::TEXTURE_2D:		return GL_TEXTURE_2D;
	case g_typ::TEXTURE_CUBE:	return GL_TEXTURE_CUBE_MAP;
	case g_typ::BYTE:			return GL_BYTE;
	case g_typ::UNSIGNED_BYTE:	return GL_UNSIGNED_BYTE;
	case g_typ::SHORT:			return GL_SHORT;
	case g_typ::UNSIGNED_SHORT:	return GL_UNSIGNED_SHORT;
	default: return GL_INVALID_ENUM;
	}
}

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
	void enableDepthTest(bool enable) override;
	void enableCullFace(bool enable) override;
	void depthMask(bool val) override;

	void clear(BufferBit bits) override;
	void setClearColor(float r, float g, float b, float a) override;
	void setViewport(int x, int y, int width, int height) override;
	void cmdDrawElements(Topology t, size_t elementLength) override;
	void cmdDrawMultiElements(Topology t, uint32_t* startIndexes, int* lengths, int multiSize) override;
	void cmdDrawArrays(Topology t, size_t elementLength) override;
};



