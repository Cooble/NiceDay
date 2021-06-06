#pragma once

namespace nd {

template <typename T>
class Pool
{
private:
	std::queue<T*> m_free_list;
	//#ifdef ND_DEBUG 
	std::vector<T*> m_allocated_list;
	//#endif
	const int m_size;
	const bool m_foreign_allocation;
	T* m_src;
public:
	Pool(const Pool&) = delete;

	Pool(int defaultSize);
	Pool(void* pointer, int defaultSize);
	~Pool();

public:
	template <typename... Args>
	T* allocate(Args&&... args);
	T* allocateNoConstructor();
	void deallocate(T* t);
	void deallocateNoDestructor(T* t);
	inline int getMaxSize() const { return m_size; }
	inline int getCurrentSize() const { return m_size - m_free_list.size(); }
	inline int getFreeSize() const { return m_free_list.size(); }
};

template <typename T>
Pool<T>::Pool(int defaultSize) : m_size(defaultSize), m_foreign_allocation(false)
{
	m_src = (T*)malloc(defaultSize * sizeof(T));
	for (int i = 0; i < defaultSize; ++i)
		m_free_list.push(m_src + i);
}

template <typename T>
Pool<T>::Pool(void* pointer, int defaultSize): m_size(defaultSize), m_foreign_allocation(true)
{
	m_src = pointer;
	for (int i = 0; i < defaultSize; ++i)
		m_free_list.push(m_src + i);
}

template <typename T>
Pool<T>::~Pool()
{
	if (!m_foreign_allocation)
		free(m_src);
}

template <typename T>
template <typename ... Args>
T* Pool<T>::allocate(Args&&... args)
{
	if (!m_free_list.empty())
	{
		auto out = m_free_list.front();
		//#ifdef ND_DEBUG
		ASSERT(std::find(m_allocated_list.begin(), m_allocated_list.end(), out) == m_allocated_list.end(),
		       "Allocation of allocated thing wtf");
		m_allocated_list.push_back(out);
		//#endif
		new(out) T(std::forward<Args>(args)...);
		m_free_list.pop();
		return out;
	}
	ND_WARN("Pool is full :(");
	return nullptr;
}

template <typename T>
T* Pool<T>::allocateNoConstructor()
{
	if (!m_free_list.empty())
	{
		auto out = m_free_list.front();
		m_free_list.pop();
		//#ifdef ND_DEBUG
		m_allocated_list.push_back(out);
		//#endif
		return out;
	}
	ND_WARN("Pool is full :(");
	return nullptr;
}

template <typename T>
void Pool<T>::deallocate(T* t)
{
	//#ifdef ND_DEBUG
	ASSERT(std::find(m_allocated_list.begin(), m_allocated_list.end(), t) != m_allocated_list.end(),
	       "Dealocation of unallocated thing");
	m_allocated_list.erase(std::find(m_allocated_list.begin(), m_allocated_list.end(), t));
	//#endif
	t->~T();
	m_free_list.push(t);
}

template <typename T>
void Pool<T>::deallocateNoDestructor(T* t)
{
	//#ifdef ND_DEBUG
	ASSERT(std::find(m_allocated_list.begin(), m_allocated_list.end(), t) != m_allocated_list.end(),
	       "Dealocation of unallocated thing");
	m_allocated_list.erase(std::find(m_allocated_list.begin(), m_allocated_list.end(), t));
	//#endif
	m_free_list.push(t);
}
}
