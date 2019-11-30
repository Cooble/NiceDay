#pragma once

/*
 * Allocates memory on big chunk of memory it has
 * no free() required
 * needs to call reset to clear whole stack
 */
class StackAllocator
{
private:
	size_t m_byte_size;
	uint8_t* m_src;
	uint8_t* m_current_pointer;
	
public:
	// todo StackAllocator(size_t byteSize,CustomAllocator alloc);
	StackAllocator(size_t byteSize);
	~StackAllocator();

	void resizeAndReset(size_t byteSize);

	// clears the content of allocator
	void reset();

	//returns a chunk of memory with specified size or nullptr if not enough memory
	void* allocate(size_t size);

};

/**
 * Works as StackAllocator
 * only difference is that allocated memory will remain untouched after first swapBuffers()
 */
class DoubleBuffStackAllocator
{
private:
	StackAllocator m_0;
	StackAllocator m_1;
	
	StackAllocator* m_current;
	StackAllocator* m_past;
public:
	// creates 2 stackAllocators with each 'byteSize' of size
	DoubleBuffStackAllocator(size_t byteSize);


	void resizeAndReset(size_t byteSize);
	// currentBuffer turns into past one,
	// pastBuffer resets and is used as current one
	void swapBuffers();

	//returns a chunk of memory with specified size or nullptr if not enough memory
	void* allocate(size_t size);

	template<typename T>
	T* allocate(T& val);
};

template <typename T>
T* DoubleBuffStackAllocator::allocate(T& val)
{
	T* p = (T*)allocate(sizeof(T));
	if (p == nullptr)
		return nullptr;
	memcpy(p, &val, sizeof(T));
	//*p = val;
	return p;
}
