#pragma once

template<typename T>
class Pool
{
private:
	std::queue<T*> m_free_list;
	const int m_size;
	const bool m_foreign_allocation;
	T* m_src;
public:
	Pool(const Pool&) = delete;

	Pool(int defaultSize);
	Pool(void* pointer,int defaultSize);
	~Pool();

public:
	T* allocate();
	void deallocate(T* t);
	inline int getMaxSize() const { return m_size; }
	inline int getCurrentSize() const { return m_free_list.size(); }
	inline int getFreeSize() const { return m_size-m_free_list.size(); }
	
};

template <typename T>
Pool<T>::Pool(int defaultSize) : m_size(defaultSize), m_foreign_allocation(false)
{
	m_src =(T*)malloc(defaultSize * sizeof(T));
	for (int i = 0; i < defaultSize; ++i)
		m_free_list.push(m_src + i);
}

template <typename T>
Pool<T>::Pool(void* pointer, int defaultSize): m_size(defaultSize),m_foreign_allocation(true)
{
	m_src = pointer;
	for (int i = 0; i < defaultSize; ++i)
		m_free_list.push(m_src + i);
}

template <typename T>
Pool<T>::~Pool()
{
	if(!m_foreign_allocation)
		free(m_src);
}

template <typename T>
T* Pool<T>::allocate()
{
	if(!m_free_list.empty())
	{
		auto out = m_free_list.front();
		m_free_list.pop();
		return out;
	}
	ND_WARN("Pool is full :(");
	return nullptr;
}

template <typename T>
void Pool<T>::deallocate(T* t)
{
	m_free_list.push(t);
}
