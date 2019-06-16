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
		inline bool empty() const { return m_size == 0; }
		void clear();
	};

	template <class T>
	FifoList<T>::FifoList(int size)
		: m_head(0), m_size(0), m_max_size(size), m_src(new T[size])
	{
	}

	template <class T>
	FifoList<T>::~FifoList()
	{
		delete[] m_src;
	}

	template <class T>
	inline void FifoList<T>::push(const T& v)
	{
		if (m_head == m_max_size)
		{
			T* newRay = new T[m_max_size * 2];
			memcpy(newRay, m_src, sizeof(T) * m_max_size);

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
	inline const T& FifoList<T>::pop()
	{
		T& t = m_src[m_head++];
		--m_size;
		if (m_size == 0)
			m_head = 0;

		return t;
	}

	template <class T>
	inline void FifoList<T>::popMode()
	{
		m_head = 0;
	}

	template <class T>
	inline void FifoList<T>::clear()
	{
		m_head = 0;
		m_size = 0;
	}
}

struct half_int
{
	inline static int getX(int i)
	{
		return ((half_int*)&i)->x;
	}
	inline static int getY(int i)
	{
		return ((half_int*)&i)->y;
	}
	union
	{
		uint32_t i;

		const struct
		{
			uint16_t x; //lsb
			uint16_t y; //msb
		};
	};

	half_int()
	{
	}

	half_int(int in) : i(in)
	{
	}

	half_int(int xx, int yy) : x((short)xx), y((short)yy)
	{
	}

	half_int plus(half_int v) const
	{
		return half_int(x + v.x, y + v.y);
	}

	half_int operator+(half_int v) const
	{
		return plus(v);
	}
	inline operator int() const
	{
		return i;
	}
	
};

inline bool operator!=(const half_int& f0, const half_int& f1)
{
	return !(f0.i == f1.i);
}
inline bool operator==(const half_int& f0, const half_int& f1)
{
	return f0.i == f1.i;
}

struct Vector2D
{
	float x, y;

	Vector2D()
		: x(0), y(0)
	{
	}

	Vector2D(float v)
		: x(v), y(v)
	{
	}

	Vector2D(float xx, float yy)
		: x(xx), y(yy)
	{
	}


	inline Vector2D operator+(const Vector2D& vector) const
	{
		return {x + vector.x, y + vector.y};
	}

	inline Vector2D operator-(const Vector2D& vector) const
	{
		return {x - vector.x, y - vector.y};
	}

	inline Vector2D operator*(const Vector2D& vector) const
	{
		return {x * vector.x, y * vector.y};
	}

	inline Vector2D operator/(const Vector2D& vector) const
	{
		return {x / vector.x, y / vector.y};
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
	return f0.x == f1.x && f0.y == f1.y;
}

inline bool operator!=(const Vector2D& f0, const Vector2D& f1)
{
	return !(f0 == f1);
}

struct iVec2D
{
	int x, y;

	iVec2D()
		: x(0), y(0)
	{
	}

	iVec2D(int v)
		: x(v), y(v)
	{
	}

	iVec2D(int xx, int yy)
		: x(xx), y(yy)
	{
	}


	inline iVec2D operator+(const iVec2D& vector) const
	{
		return { x + vector.x, y + vector.y };
	}

	inline iVec2D operator-(const iVec2D& vector) const
	{
		return { x - vector.x, y - vector.y };
	}

	inline iVec2D operator*(const iVec2D& vector) const
	{
		return { x * vector.x, y * vector.y };
	}

	inline iVec2D operator/(const iVec2D& vector) const
	{
		return { x / vector.x, y / vector.y };
	}

	inline int length() const
	{
		return abs(x)+abs(y);
	}

	inline void plus(const iVec2D& vector)
	{
		x += vector.x;
		y += vector.y;
	}

	inline void minus(const iVec2D& vector)
	{
		x -= vector.x;
		y -= vector.y;
	}

	inline void multiply(const iVec2D& vector)
	{
		x *= vector.x;
		y *= vector.y;
	}

	inline void divide(const iVec2D& vector)
	{
		x /= vector.x;
		y /= vector.y;
	}
};

inline bool operator==(const iVec2D& f0, const iVec2D& f1)
{
	return f0.x == f1.x && f0.y == f1.y;
}

inline bool operator!=(const iVec2D& f0, const iVec2D& f1)
{
	return !(f0 == f1);
}

struct Rect
{
private:
	inline bool collideInner(const Rect& r) const {
		return
			contains({ r.x, r.y }) ||
			contains({ r.x +r.w, r.y }) ||
			contains({ r.x, r.y +r.h}) ||
			contains({ r.x +r.w, r.y +r.h});
	}
public:
	union
	{
		Vector2D pos;
		struct
		{
		float x, y;
		};
	};
	union
	{
		Vector2D size;
		struct
		{
			float w, h;
		};
	};

	Rect(float x, float y, float width, float height)
		: pos({x, y}), size({width,height})
	{
	}
	Rect(const Vector2D& pos,const Vector2D& size)
		: pos(pos), size(size)
	{
	}
	inline bool contains(const Vector2D& p)  const {

		return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
	}
	inline bool collides(const Rect& rec) const
	{
		return collideInner(rec) || rec.collideInner(*this);
	}

};
template <typename T>
constexpr T ipow(T num, unsigned int pow)
{
	return (pow >= sizeof(unsigned int) * 8) ? 0 :
		pow == 0 ? 1 : num * ipow(num, pow - 1);
}

#include <unordered_map>
template<typename Key, typename T, T value = T()>
class defaultable_map :public std::unordered_map<Key, T>
{
public:
	// inherit std::unordered_map constructors
	using std::unordered_map<Key, T>::unordered_map;

	T & operator[](const Key & key)
	{
		auto it = find(key);

		if (it == end())
		{
			// insert default value
			auto result = insert(std::make_pair(key, value));
			it = result.first;
		}

		return it->second;
	}
	bool contains(const Key& key)
	{
		return find(key) != end();
	}
};
