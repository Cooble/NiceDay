#include "ndpch.h"
#include "StackAllocator.h"

StackAllocator::StackAllocator(size_t byteSize)
	: m_byte_size(byteSize)
{
	ASSERT(m_byte_size, "Invalid  byteSize");
	m_src = (uint8_t*)malloc(m_byte_size);
	reset();
}

StackAllocator::~StackAllocator()
{
	free(m_src);
}

void StackAllocator::resizeAndReset(size_t byteSize)
{
	ASSERT(byteSize, "Invalid  byteSize");
	free(m_src);
	m_byte_size = byteSize;
	m_src = (uint8_t*)malloc(m_byte_size);
}

void StackAllocator::reset()
{
	m_current_pointer = m_src;
#if STACK_ALLOC_MEM_CLEAR_ENABLE
	memset(m_src, 0, m_byte_size);
#endif
}

void* StackAllocator::allocate(size_t size)
{
	if (m_current_pointer + size > m_src + m_byte_size)
		return nullptr;
	auto out = m_current_pointer;
	m_current_pointer += size;
	return out;
}

DoubleBuffStackAllocator::DoubleBuffStackAllocator(size_t byteSize):
m_0(byteSize),m_1(byteSize)
{
	m_current = &m_0;
	m_past = &m_1;
}

void DoubleBuffStackAllocator::resizeAndReset(size_t byteSize)
{
	m_0.resizeAndReset(byteSize);
	m_1.resizeAndReset(byteSize);
}

void DoubleBuffStackAllocator::swapBuffers()
{
	m_past->reset();
	
	auto m = m_current;
	m_current = m_past;
	m_past = m;
}

void* DoubleBuffStackAllocator::allocate(size_t size)
{
	return m_current->allocate(size);
}
