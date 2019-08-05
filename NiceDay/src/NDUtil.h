#pragma once
#include <bitset>

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


	class Bitset
	{
	private:
		std::vector<std::bitset<32>> m_bits;
		size_t m_last_size=0;

	public:
		Bitset() = default;

		inline void resize(size_t bits)
		{
			if(bits%32!=0)
				bits = (bits / 32) * 32 + 32;
			if (m_bits.size() < (bits/32))
				m_bits.resize(bits / 32);
		}
		inline bool get(size_t index)
		{
			return m_bits[index >> 5][index&((1 << 5) - 1)];
		}
		inline void set(size_t index,bool val)
		{
			m_bits[index >> 5][index&((1 << 5) - 1)]=val;
		}
		inline bool operator[](size_t index)
		{
			return get(index);
		}
		inline void push_back(bool val)
		{
			++m_last_size;
			resize(m_last_size);
			set(m_last_size - 1,val);
		}
		inline auto& getSource()const { return m_bits; }
		inline size_t size() { return m_bits.size() * 32; }
	};



	template <typename T>
	static void eraseKeepPacked(std::vector<T>& t,size_t indexToErase)
	{
		auto& tt = t[t.size() - 1];
		t[indexToErase] = tt;
		t.erase(t.end() - 1);
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
