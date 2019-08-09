#pragma once
#include "Renderer.h"

#define Gcon GContext::get()

enum BufferBit : unsigned int
{
	COLOR_BUFFER_BIT = GL_COLOR_BUFFER_BIT,
	DEPTH_BUFFER_BIT = GL_DEPTH_BUFFER_BIT,
};
enum class Blend :unsigned int
{
	ZERO = GL_ZERO,
	ONE = GL_ONE,
	SRC_COLOR = GL_SRC_COLOR,
	ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
	SRC_ALPHA = GL_SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
	DST_ALPHA = GL_DST_ALPHA,
	ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
	DST_COLOR = GL_DST_COLOR,
	ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
	CONSTANT_COLOR = GL_CONSTANT_COLOR,
	ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
	CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
	NONE
};
enum class BlendEquation :unsigned int
{
	FUNC_ADD = GL_FUNC_ADD,
	FUNC_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
	FUNC_SUBTRACT = GL_FUNC_SUBTRACT,
	MIN = GL_MIN,
	MAX = GL_MAX,
	NONE
};

class GContext
{
private:
	inline static GContext* s_context = nullptr;
public:
	inline static GContext& get()
	{
		return *s_context;
	}

	static void init(GraphicsAPI api);

	virtual ~GContext() = default;

	virtual void enableBlend() = 0;
	virtual void disableBlend() = 0;

	virtual void setBlendEquation(BlendEquation e)=0;
	virtual void setBlendFuncSeparate(Blend src, Blend dst, Blend srcA, Blend dstA)=0;
	virtual void setBlendFunc(Blend src, Blend dst)=0;
	virtual void setBlendConstant(float r, float g, float b, float a) = 0;

	virtual void clear(BufferBit bits) = 0;
	virtual void setClearColor(float r, float g, float b, float a) = 0;
	virtual void setViewport(int x, int y, int width, int height) = 0;
	inline void setViewport(int width, int height) { setViewport(0, 0, width, height); }
	
};
