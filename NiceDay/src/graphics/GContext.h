#pragma once
#include "Renderer.h"
#include "glad/glad.h"

#define Gcon GContext::get()

typedef unsigned int BufferBit;

namespace BuffBit {
	constexpr BufferBit COLOR = GL_COLOR_BUFFER_BIT;
	constexpr BufferBit DEPTH = GL_DEPTH_BUFFER_BIT;
	constexpr BufferBit STENCIL = GL_STENCIL_BUFFER_BIT;
}


enum class Blend : uint32_t
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
enum class BlendEquation : uint32_t
{
	FUNC_ADD = GL_FUNC_ADD,
	FUNC_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
	FUNC_SUBTRACT = GL_FUNC_SUBTRACT,
	MIN = GL_MIN,
	MAX = GL_MAX,
	NONE
};
enum class StencilFunc :uint32_t
{
	NEVER = GL_NEVER,
	LESS=GL_LESS,
	LEQUAL=GL_LEQUAL,
	GREATER=GL_GREATER,
	GEQUAL=GL_GEQUAL,
	EQUAL=GL_EQUAL,
	NOTEQUAL=GL_NOTEQUAL,
	ALWAYS=GL_ALWAYS
};
enum class StencilOp :uint32_t
{
	KEEP = GL_KEEP,
	ZERO= GL_ZERO,
	REPLACE= GL_REPLACE,
	INCR= GL_INCR,
	INCR_WRAP= GL_INCR_WRAP,
	DECR= GL_DECR,
	DECR_WRAP= GL_DECR_WRAP,
};
enum class Topology : uint32_t
{
	LINES=GL_LINES,
	TRIANGLES = GL_TRIANGLES,
	TRIANGLE_FAN = GL_TRIANGLE_FAN,
	TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
	TRIANGLE_STRIP= GL_TRIANGLE_STRIP,
};
enum class RenderStage : uint32_t
{
	VERTEX = 1,
	FRAGMENT = 2,
	GEOMETRY = 4
};
enum class g_typ : uint32_t
{
	BYTE,
	UNSIGNED_BYTE,
	SHORT,
	UNSIGNED_SHORT,
	UNSIGNED_INT,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	INT,
	IVEC2,
	IVEC3,
	IVEC4,
	MAT3,
	MAT4,
	TEXTURE_2D,
	TEXTURE_CUBE,

	INVALID
};

enum class VertexType : uint32_t
{
	POS,
	NORMAL,
	UV,

	INVALID
};

namespace GTypes {
	constexpr bool isIType(g_typ type)
	{
		switch (type) {
		case g_typ::UNSIGNED_INT:
		case g_typ::INT:
		case g_typ::IVEC2:
		case g_typ::IVEC3:
		case g_typ::IVEC4:
		case g_typ::UNSIGNED_BYTE:
		case g_typ::SHORT:
		case g_typ::UNSIGNED_SHORT:
			return true;

		default:return false;
		}
	}
	constexpr bool isTexture(g_typ type)
	{
		return type == g_typ::TEXTURE_2D || type == g_typ::TEXTURE_CUBE;
	}
	constexpr int getCount(g_typ type)
	{
		switch(type)
		{
		case g_typ::BYTE:
		case g_typ::UNSIGNED_BYTE:
		case g_typ::SHORT:
		case g_typ::UNSIGNED_SHORT:
		case g_typ::UNSIGNED_INT:
		case g_typ::FLOAT:
		case g_typ::INT: 
		case g_typ::MAT3:
		case g_typ::MAT4:
		case g_typ::TEXTURE_2D:
		case g_typ::TEXTURE_CUBE:
			return 1;
		case g_typ::IVEC2:
		case g_typ::VEC2:
			return 2;
		case g_typ::VEC3:
		case g_typ::IVEC3:
			return 3;
		case g_typ::VEC4:
		case g_typ::IVEC4:
			return 4;
		default: return 0;
		}
	}
	// base type of vector e.g. (vec2 is float, ivec3 is int)
	// otherwise return argument
	constexpr g_typ getBase(g_typ type)
	{
		switch (type)
		{
		case g_typ::VEC2:
		case g_typ::VEC3:
		case g_typ::VEC4:
			return g_typ::FLOAT;
		case g_typ::IVEC2:
		case g_typ::IVEC3:
		case g_typ::IVEC4:
			return g_typ::INT;
		default: return type;
		}
	}
	constexpr g_typ getType(std::string_view view)
	{
		if (view == "float")
			return g_typ::FLOAT;
		if (view == "vec2")
			return g_typ::VEC2;
		if (view == "vec3")
			return g_typ::VEC3;
		if (view == "vec4")
			return g_typ::VEC4;
		if (view == "mat3")
			return g_typ::MAT3;
		if (view == "mat4")
			return g_typ::MAT4;

		if (view == "int")
			return g_typ::INT;
		if (view == "ivec2")
			return g_typ::IVEC2;
		if (view == "ivec3")
			return g_typ::IVEC3;
		if (view == "ivec4")
			return g_typ::IVEC4;
		if (view == "uint")
			return g_typ::UNSIGNED_INT;
		if (view == "sampler2D")
			return g_typ::TEXTURE_2D;
		if (view == "samplerCube")
			return g_typ::TEXTURE_CUBE;
		return g_typ::INVALID;
	}
	constexpr const char* getName(g_typ type)
	{
		switch (type) {
		
		case g_typ::UNSIGNED_INT: return "uint";
		case g_typ::FLOAT: return "float";
		case g_typ::VEC2: return "vec2";
		case g_typ::VEC3: return "vec3";
		case g_typ::VEC4: return "vec4";
		case g_typ::INT: return "int";
		case g_typ::IVEC2: return "ivec2";
		case g_typ::IVEC3: return "ivec3";
		case g_typ::IVEC4: return "ivec4";
		case g_typ::MAT3: return "mat3";
		case g_typ::MAT4: return "mat4";
		case g_typ::TEXTURE_2D: return "sampler2D";
		case g_typ::TEXTURE_CUBE: return "samplerCube";
		default:
			return "INVALID TYPE";
		}
	}
	constexpr static int GTYPE_SIZES[]
	{
				1,		//	BYTE,
				1,		//UNSIGNED_BYTE,
				2,		//SHORT,
				2,		//UNSIGNED_SHORT,
				4,		//UNSIGNED_INT,
				4,		//FLOAT,
				4 * 2,	//VEC2,
				4 * 3,	//VEC3,
				4 * 4,	//VEC4,
				4,		//INT,
				4 * 2,	//IVEC2,
				4 * 3,	//IVEC3,
				4 * 4,	//IVEC4,
				4 * 3 * 3,	//MAT3,
				4 * 4 * 4,	//MAT4,
				4,		//TEXTURE,
				4,		//TEXTURE_CUBE,
				0		//INVALID
	};
	constexpr int getSize(g_typ type)
	{
		return GTYPE_SIZES[(int)type];
	}
	constexpr g_typ setCount(g_typ base,int count)
	{
		switch (base) {
		case g_typ::FLOAT:
			return (g_typ)((uint32_t)base + count-1);
		case g_typ::INT:
			return (g_typ)((uint32_t)base + count - 1);
		default:
			ASSERT(false,"invalid base type");
			return g_typ::INVALID;
		}
	}

	
}

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
	virtual void enableDepthTest(bool enable) = 0;
	virtual void enableCullFace(bool enable) = 0;
	virtual void enableStencilTest(bool enable) = 0;
	virtual void depthMask(bool val) = 0;
	virtual void stencilMask(uint8_t mask) = 0;
	
	// controls which elements should be rendered or discarded,
	// does not update stencil buffer
	// value -> reference value to which the content of stencil buffer is compared
	// mask -> both values ANDed with this mask before test 
	virtual void stencilFunc(StencilFunc func, int value, uint32_t mask=0xFF) = 0;

	// how to update stencil buffer
	// stfails -> when stencil test fails
	// dtfails -> when depth test fails
	// dtpass -> when both stencil and depth pass
	virtual void stencilOp(StencilOp stfails, StencilOp dtfails,StencilOp dtpass) = 0;
	

	virtual void clear(BufferBit bits) = 0;
	virtual void setClearColor(float r, float g, float b, float a) = 0;
	virtual void setViewport(int x, int y, int width, int height) = 0;
	void setViewport(int width, int height) { setViewport(0, 0, width, height); }
	virtual void cmdDrawElements(Topology t, size_t elementLength)=0;
	virtual void cmdDrawMultiElements(Topology t, uint32_t* startIndexes, int* lengths, int multiSize) =0;
	virtual void cmdDrawArrays(Topology t, size_t elementLength)=0;
	
	void setClearColor(const glm::vec4& c) { setClearColor(c.r, c.g, c.b, c.a); }
};
