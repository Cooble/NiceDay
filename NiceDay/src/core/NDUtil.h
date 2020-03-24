#pragma once
#include "ndpch.h"
#include <bitset>

namespace NDUtil
{
	static void splitString(const std::string& line, std::vector<std::string>& words, const std::string& divider = " \n\t")
	{
		auto beginIdx = line.find_first_not_of(divider, 0);

		while (beginIdx != std::string::npos)
		{
			auto endIdx = line.find_first_of(divider, beginIdx);
			if (endIdx != std::string::npos)
			{
				words.push_back(line.substr(beginIdx, endIdx - beginIdx));
				beginIdx = line.find_first_not_of(divider, endIdx);
			}
			else {
				words.push_back(line.substr(beginIdx));
				break;
			}

		}

	}
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
		size_t m_last_size = 0;

	public:
		Bitset() = default;

		//ensures minimum capacity to be bits size
		inline void resize(size_t bits)
		{
			if (m_last_size >= bits)
				return;
			m_last_size = bits;
			
			if ((bits % 32)!=0)
				bits = (bits / 32) * 32 + 32;
			if (m_bits.size() < (bits / 32))
				m_bits.resize(bits / 32);
		}
		
		inline void resizeHard(size_t bits)
		{
			m_last_size = bits;

			if ((bits % 32) != 0)
				bits = (bits / 32) * 32 + 32;
			if (m_bits.size() < (bits / 32))
				m_bits.resize(bits / 32);
		}

		inline bool get(size_t index) const
		{
			ASSERT((index >> 5) < m_bits.size(), "Invalid index");
			return m_bits[index >> 5][index & ((1 << 5) - 1)];
		}

		inline void set(size_t index, bool val)
		{
			ASSERT((index >> 5) < m_bits.size(), "Invalid index");
			m_bits[index >> 5][index & ((1 << 5) - 1)] = val;
		}

		inline bool operator[](size_t index) const
		{
			return get(index);
		}

		inline void push_back(bool val)
		{
			resize(m_last_size+1);
			set(m_last_size - 1, val);
		}

		inline auto& getSource() const { return m_bits; }
		inline size_t bitSize() const { return m_bits.size() * 32; }

		inline size_t byteSize() const
		{
			return m_bits.size() * sizeof(std::bitset<32>);
		}

		inline void write(std::ostream& stream) const
		{
			int arraySize = m_bits.size();
			stream.write((char*)&arraySize, sizeof(int));
			stream.write((char*)&m_last_size, sizeof(int));
			stream.write((char*)m_bits.data(), byteSize());
		}

		inline void read(std::istream& stream)
		{
			int arraySize = 0;
			stream.read((char*)&arraySize, sizeof(int));
			stream.read((char*)&m_last_size, sizeof(int));

			m_bits.resize(arraySize);
			stream.read((char*)m_bits.data(), byteSize());
		}
	};


	template <typename T>
	static void eraseKeepPacked(std::vector<T>& t, size_t indexToErase)
	{
		auto& tt = t[t.bitSize() - 1];
		t[indexToErase] = tt;
		t.erase(t.end() - 1);
	}
}

struct half_int
{
	inline static std::string toString(half_int i)
	{
		return std::to_string(i) + ":[" + std::to_string(i.x) + ", " + std::to_string(i.y) + "]";
	}
	

	inline static int X(int i)
	{
		return ((half_int*)&i)->x;
	}

	inline static int Y(int i)
	{
		return ((half_int*)&i)->y;
	}

	union
	{
		int i;

		const struct
		{
			short x; //lsb
			short y; //msb
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
inline std::ostream & operator<<(std::ostream& stream,const half_int& v) {
	auto s = half_int::toString(v);
	stream.write(s.c_str(), s.size());
	return stream;
}

struct quarter_int
{
	inline static int X(int i)
	{
		return ((quarter_int*)&i)->x;
	}

	inline static int Y(int i)
	{
		return ((quarter_int*)&i)->y;
	}

	inline static int Z(int i)
	{
		return ((quarter_int*)&i)->z;
	}

	inline static int W(int i)
	{
		return ((quarter_int*)&i)->w;
	}

	union
	{
		uint32_t i;

		const struct
		{
			uint8_t x; //lsb
			uint8_t y; //msb
			uint8_t z; //msb
			uint8_t w; //msb
		};
	};

	quarter_int()
	{
	}

	quarter_int(int in) : i(in)
	{
	}

	quarter_int(int xx, int yy, int zz, int ww) : x((char)xx), y((char)yy), z((char)zz), w((char)ww)
	{
	}

	quarter_int plus(quarter_int v) const
	{
		return quarter_int(x + v.x, y + v.y, z + v.z, w + v.w);
	}

	quarter_int operator+(quarter_int v) const
	{
		return plus(v);
	}

	inline operator int() const
	{
		return i;
	}
};
inline half_int operator*(const half_int& f0, const int& f1)
{
	return half_int(f0.x * f1, f0.y * f1);
}


inline bool operator!=(const half_int& f0, const half_int& f1)
{
	return !(f0.i == f1.i);
}

inline bool operator==(const half_int& f0, const half_int& f1)
{
	return f0.i == f1.i;
}

inline bool operator!=(const half_int& f0, const int& f1)
{
	return !(f0.i == f1);
}

inline bool operator==(const half_int& f0, const int& f1)
{
	return f0.i == f1;
}

inline bool operator!=(const int& f0, const half_int& f1)
{
	return !(f0 == f1.i);
}

inline bool operator==(const int& f0, const half_int& f1)
{
	return f0 == f1.i;
}

inline bool operator!=(const quarter_int& f0, const quarter_int& f1)
{
	return !(f0.i == f1.i);
}

inline bool operator==(const quarter_int& f0, const quarter_int& f1)
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
		return {x + vector.x, y + vector.y};
	}

	inline iVec2D operator-(const iVec2D& vector) const
	{
		return {x - vector.x, y - vector.y};
	}

	inline iVec2D operator*(const iVec2D& vector) const
	{
		return {x * vector.x, y * vector.y};
	}

	inline iVec2D operator/(const iVec2D& vector) const
	{
		return {x / vector.x, y / vector.y};
	}

	inline int length() const
	{
		return abs(x) + abs(y);
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
	return (pow >= sizeof(unsigned int) * 8) ? 0 : pow == 0 ? 1 : num * ipow(num, pow - 1);
}

#include <unordered_map>

template <typename Key, typename T, T value = T()>
class defaultable_map : public std::unordered_map<Key, T>
{
public:
	// inherit std::unordered_map constructors
	using std::unordered_map<Key, T>::unordered_map;

	T& operator[](const Key& key)
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

template <typename Key, typename T>
class defaultable_map_other : public std::unordered_map<Key, T>
{
public:
	// inherit std::unordered_map constructors
	using std::unordered_map<Key, T>::unordered_map;

	T& operator[](const Key& key)
	{
		auto it = find(key);

		if (it == end())
		{
			// insert default value
			auto result = insert(std::make_pair(key, T()));
			it = result.first;
		}

		return it->second;
	}

	bool contains(const Key& key)
	{
		return find(key) != end();
	}
};

inline float randFloat(float size = 1) { return std::rand() % 1000 / 1000.f * size; }
inline float randDispersedFloat(float absSize = 1) { return (std::rand() % 1000 / 1000.f - 0.5f) * absSize * 2; }

class JobAssignment
{
public:
	static const int64_t JOB_SUCCESS = 0;
	static const int64_t JOB_FAILURE = std::numeric_limits<int64_t>::max();
	std::atomic<int64_t> m_main;
	std::atomic<int64_t> m_worker;
public:
	std::atomic <int64_t> m_variable;

public:
	JobAssignment() { m_main.store(0); m_worker.store(0); m_variable.store(0); }
	JobAssignment(const JobAssignment& a)
	{
		m_main.store(a.m_main);
		m_worker.store(a.m_worker);
		m_variable.store(a.m_variable);
	}
	JobAssignment& operator=(const JobAssignment& a)
	{
		m_main.store(a.m_main);
		m_worker.store(a.m_worker);
		m_variable.store(a.m_variable);
		return *this;
	}

	inline void assign() { ++m_main; }

	inline void markDone()
	{
		ASSERT(m_worker < m_main, "marking Done Job which has not been assigned.");
		++m_worker;
	}

	inline bool isDone() const { return m_main == m_worker; }

	inline void reset()
	{
		ASSERT(isDone(), "Cannot reset when job is pending");
		m_main = 0;
		m_worker = 0;
		m_variable = 0;
	}

	inline bool isSuccess() const { return m_variable == JOB_SUCCESS; }
	inline bool isFailure() const { return m_variable != JOB_SUCCESS; }
};

typedef JobAssignment* JobAssignmentP;


template <typename WorkAssignment>
class Worker
{
private:
	std::thread m_t;
	std::atomic_bool m_is_buffering_assignments = false;
	std::atomic_bool m_is_stopped = true;
	std::atomic_bool m_is_running = false;
	std::mutex m_queue_mutex;
	std::queue<WorkAssignment> m_queue; // Have I found everybody fun assignment to do today?
	std::condition_variable m_wait_condition_variable;
private:
	void runInner()
	{
		m_is_running = true;
		m_is_stopped = false;
		init();
		std::mutex waitMutex;
		std::unique_lock<std::mutex> loopLock(waitMutex);
		std::vector<WorkAssignment> bufferAssigns;
		while (m_is_running)
		{
			m_wait_condition_variable.wait(loopLock);

			bool runAgain = true;
			while (runAgain&&m_is_running)
			{
				{
					std::unique_lock<std::mutex> guard(m_queue_mutex);
					bufferAssigns.resize(m_queue.size()); //this is lame too much memory alloc and stuff
					int index = 0;
					while (!m_queue.empty())
					{
						bufferAssigns[index++] = m_queue.front();
						m_queue.pop();
					}
				}
				proccessAssignments(bufferAssigns);
				{
					std::unique_lock<std::mutex> guard(m_queue_mutex);
					runAgain = !m_queue.empty() && m_is_running;
				}
			}
		}
		deInit();
		m_is_stopped = true;
	}

public:
	virtual ~Worker()
	{
		if (!m_is_running)
			return;

		m_is_running = false;
		m_wait_condition_variable.notify_one();

		m_t.join();
		while (!m_is_stopped);
	}


	// will create separate working thread and return immediately
	inline void start()
	{
		m_t = std::thread(&Worker::runInner, this);
		//t.detach(); //fly daemon, fly ,nope
	}

	// will notify work thread to stop
	// to check if work thread is truly killed check isStopped()
	inline void stop()
	{
		m_is_running = false;
		m_wait_condition_variable.notify_one();
	}

	// called by work thread after start before processing assignments
	inline virtual void init()
	{
	};

	inline virtual void deInit()
	{
	};

	// dont forget to mark the job as done
	virtual void proccessAssignments(std::vector<WorkAssignment>& assignments) = 0;

	inline bool isStopped()
	{
		return m_is_stopped;
	}

	void assignWork(const WorkAssignment* assignments, int size)
	{
		std::lock_guard<std::mutex> guard(m_queue_mutex);
		for (int i = 0; i < size; ++i)
			m_queue.push(assignments[i]);
		if (!m_is_buffering_assignments)
			m_wait_condition_variable.notify_one();
	}

	inline void beginBuffering() { m_is_buffering_assignments = true; }

	inline void flushBuffering()
	{
		m_is_buffering_assignments = false;
		m_wait_condition_variable.notify_one();
	}

	inline void assignWork(const WorkAssignment& assignment)
	{
		assignWork(&assignment, 1);
	}
};

