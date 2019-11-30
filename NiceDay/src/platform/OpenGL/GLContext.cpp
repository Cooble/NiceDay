#include "ndpch.h"
#include "GLContext.h"
#include "GLRenderer.h"

void GLContext::enableBlend()
{
	GLCall(glEnable(GL_BLEND));

	return;
	if (!m_blend_enabled)
		GLCall(glEnable(GL_BLEND));
	m_blend_enabled = true;
}

void GLContext::disableBlend()
{
	GLCall(glDisable(GL_BLEND));

	return;
	if (m_blend_enabled)
		GLCall(glDisable(GL_BLEND));
	m_blend_enabled = false;
}

void GLContext::setBlendEquation(BlendEquation e)
{
	GLCall(glBlendEquation((unsigned int)e));
	return;
	if(m_current_blend_data.blendEquation!=e)
		GLCall(glBlendEquation((unsigned int)e));
	m_current_blend_data.blendEquation = e;
}

void GLContext::setBlendFuncSeparate(Blend src, Blend dst, Blend srcA, Blend dstA)
{
	GLCall(glBlendFuncSeparate((unsigned int)src, (unsigned int)dst, (unsigned int)srcA, (unsigned int)dstA));

	return;
	if (
		m_current_blend_data.blends[0] == src
		&& m_current_blend_data.blends[1] == dst
		&& m_current_blend_data.blends[2] == srcA
		&&m_current_blend_data.blends[3] == dstA)
	{

	}
	else
		GLCall(glBlendFuncSeparate((unsigned int)src, (unsigned int)dst, (unsigned int)srcA, (unsigned int)dstA));
	auto& blends = m_current_blend_data.blends;
	blends[0] = src;
	blends[1] = dst;
	blends[2] = srcA;
	blends[3] = dstA;
}

void GLContext::setBlendFunc(Blend src, Blend dst)
{
	GLCall(glBlendFunc((unsigned int)src, (unsigned int)dst));
	return;
	if(
		m_current_blend_data.blends[0]==src
		&&m_current_blend_data.blends[1]==dst
		&&m_current_blend_data.blends[2]==Blend::NONE
		&&m_current_blend_data.blends[3] == Blend::NONE)
	{
		
	}else
		GLCall(glBlendFunc((unsigned int)src, (unsigned int)dst));
	auto& blends = m_current_blend_data.blends;
	blends[0] = src;
	blends[1] = dst;
	blends[2] = Blend::NONE;
	blends[3] = Blend::NONE;
}

void GLContext::setBlendConstant(float r, float g, float b, float a)
{
	GLCall(glBlendColor(r, g, b, a));
	return;

	if (
		m_current_blend_data.constant[0] == r
		&& m_current_blend_data.constant[1] == g
		&& m_current_blend_data.constant[2] == b
		&&m_current_blend_data.constant[3] ==  a)
	{

	}
	else
		GLCall(glBlendColor(r, g, b, a));
	auto& colors = m_current_blend_data.constant;
	colors[0] = r;
	colors[1] = g;
	colors[2] = b;
	colors[3] = a;

}

void GLContext::enableDepthTest(bool enable)
{
	if (enable) {
		GLCall(glEnable(GL_DEPTH_TEST));
	}
	else
		GLCall(glDisable(GL_DEPTH_TEST));

}

void GLContext::clear(BufferBit bits)
{
	GLCall(glClear(bits));
}

void GLContext::setClearColor(float r, float g, float b, float a)
{
	GLCall(glClearColor(r, g, b, a));
	return;

	if (
		m_clearColor[0] != r
		|| m_clearColor[1] != g
		|| m_clearColor[2] != b
		|| m_clearColor[3] != a) {

		GLCall(glClearColor(r, g, b, a));
	}

	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void GLContext::setViewport(int x, int y, int width, int height)
{
	GLCall(glViewport(x, y, width, height));
	return;
	if (
		m_viewport[0] != x
		|| m_viewport[1] != y
		|| m_viewport[2] != width
		|| m_viewport[3] != height) {

		GLCall(glViewport(x,y,width,height));
	}

	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = width;
	m_viewport[3] = height;
}

void GLContext::cmdDrawElements(Topology t, size_t elementLength)
{
	GLCall(glDrawElements((uint32_t)t,elementLength,GL_UNSIGNED_INT, nullptr));
}

void GLContext::cmdDrawMultiElements(Topology t, uint32_t* startIndexes, int* lengths, int multiSize)
{
	glMultiDrawElements((uint32_t)t, lengths, GL_UNSIGNED_INT, (void**)startIndexes, multiSize);//fuk of
}

void GLContext::cmdDrawArrays(Topology t, size_t elementLength)
{
	GLCall(glDrawArrays((uint32_t)t,0, elementLength));
}


