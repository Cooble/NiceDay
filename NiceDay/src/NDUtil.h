#pragma once

namespace NDUtil
{
	//List is dynamically allocated, will get bigger if more items are pushed
	//1. push() items to list
	//2. call popMode()
	//3. pop() items from list till isEmpty()
	//4. call clear()
	//5. rinse and repeat
	template <class T>
	class FifoList
	{
	private:
		int m_head;
		int m_size;
		int m_max_size;
		T* m_src;
	public:
		FifoList(int size);
		~FifoList();

		void push(const T&);
		const T& pop();
		void popMode();
		inline int getSize() const { return m_size; }
		inline bool empty() const { return getSize() == 0; }
		void clear();


	};

	template <class T>
	FifoList<T>::FifoList(int size)
		:m_head(0), m_size(0), m_max_size(size), m_src(new T[size])
	{

	}

	template <class T>
	FifoList<T>::~FifoList()
	{
		delete[] m_src;
	}

	template <class T>
	void FifoList<T>::push(const T& v)
	{
		if (m_head == m_max_size)
		{

			T* newRay = new T[m_max_size * 2];
			memcpy(newRay, m_src, sizeof(T)*m_max_size);

			delete[] m_src;
			m_src = newRay;
			m_max_size *= 2;

			//memcpy
		}
		m_src[m_head] = v;

		++m_head;
		++m_size;


	}

	template <class T>
	const T& FifoList<T>::pop()
	{
		T& t = m_src[m_head++];
		--m_size;
		if (m_size == 0)
			m_head = 0;

		return t;
	}

	template <class T>
	void FifoList<T>::popMode()
	{
		m_head = 0;
	}

	template <class T>
	void FifoList<T>::clear()
	{
		m_head = 0;
		m_size = 0;
	}

}
struct half_int
{
	union
	{
		uint32_t i;
		struct
		{
			uint16_t x;//lsb
			uint16_t y;//msb
		};
	};
	half_int(){}
	half_int(int in) :i(in) {}
	half_int(int xx, int yy) :x((short)xx), y((short)yy) {}
	half_int plus(half_int v) const
	{
		return half_int(x + v.x, y + v.y);
	}
	half_int operator+(half_int v) const
	{
		return plus(v);
	}

};
struct Vector2D
{
	float x, y;
	Vector2D()
		:x(0), y(0)
	{}
	Vector2D(float v)
		:x(v), y(v)
	{}
	Vector2D(float xx, float yy)
		:x(xx), y(yy)
	{}

	
	inline Vector2D operator+(const Vector2D& vector) const
	{
		return { x + vector.x,y + vector.y };
	}
	inline Vector2D operator-(const Vector2D& vector) const
	{
		return { x - vector.x,y - vector.y };
	}
	inline Vector2D operator*(const Vector2D& vector) const
	{
		return { x * vector.x,y * vector.y };
	}
	inline Vector2D operator/(const Vector2D& vector) const
	{
		return { x / vector.x,y / vector.y };
	}

	inline float length() const
	{
		return std::sqrt(x * x + y * y);
	}
	inline float lengthSquared() const
	{
		return x * x + y * y;
	}
	inline void plus(const Vector2D& vector)
	{
		x += vector.x;
		y += vector.y;
	}
	inline void minus(const Vector2D& vector)
	{
		x -= vector.x;
		y -= vector.y;
	}
	inline void multiply(const Vector2D& vector)
	{
		x *= vector.x;
		y *= vector.y;
	}
	inline void divide(const Vector2D& vector)
	{
		x /= vector.x;
		y /= vector.y;
	}

};
inline bool operator==(const Vector2D& f0, const Vector2D& f1)
{
	return f0.x == f1.x&&f0.y == f1.y;
}
inline bool operator!=(const Vector2D& f0, const Vector2D& f1)
{
	return !(f0 == f1);
}
