#pragma once
namespace nd {

// each tick memory will be set to 0
#define STACK_ALLOC_MEM_CLEAR_ENABLE 1
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

	// @return a chunk of memory with specified size or nullptr if not enough memory
	void* allocate(size_t size);

	//==================SPECIALIZED ALLOCATIONS==================

	// @param c null-terminated
	inline std::string_view allocateString(const char* c)
	{
		int strl = strlen(c) + 1; //include terminator char
		auto p = (char*)allocate(strl);
		memcpy(p, c, strl);
		return std::string_view(p);
	}

	inline std::string_view allocateString(const std::string& c)
	{
		return allocateString(c.c_str());
	}


	// constructs copy of the object in allocated space
	// @return nullptr if no buffer is full
	template <typename T>
	T* allocateObject(T&& val)
	{
		T* p = (T*)allocate(sizeof(T));
		if (p == nullptr)
			return nullptr;
		memcpy(p, &std::forward<T>(val), sizeof(T));
		return p;
	}

	// constructs the object in allocated space
	// @return nullptr if no buffer is full
	template <typename T, typename... Args>
	T* emplace(Args&&... args)
	{
		T* p = (T*)allocate(sizeof(T));
		if (p == nullptr)
			return nullptr;
		*p = T(std::forward<Args>(args)...);
		return p;
	}

	inline size_t getCurrentStackSize()
	{
		return (size_t)(m_current_pointer - m_src);
	}
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

	// @return a chunk of memory with specified size or
	// @return nullptr if not enough memory
	void* allocate(size_t size);

	inline size_t getCurrentStackSize()
	{
		return m_current->getCurrentStackSize();
	}

	//==================SPECIALIZED ALLOCATIONS==================

	// @param c must be null-terminated
	template <typename StringOrCharP>
	inline std::string_view allocateString(StringOrCharP&& c)
	{
		return m_current->allocateString(std::forward<StringOrCharP>(c));
	}

	// constructs copy of the object in allocated space
	// @return nullptr if no buffer is full
	template <typename T>
	inline T* allocateObject(T&& val)
	{
		return m_current->allocateObject(std::forward<T>(val));
	}

	// constructs the object in allocated space
	// @return nullptr if no buffer is full
	template <typename T, typename... Args>
	inline T* emplace(Args&&... args)
	{
		return m_current->emplace<T>(std::forward<Args>(args)...);
	}
};
}
