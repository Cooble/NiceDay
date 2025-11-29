#pragma once
#include <charconv>

#include "ndpch.h"


namespace nd::internal {
void* allocateMeeh(size_t n);
void* allocateMeehStandard(size_t n);
void deallocateMeehStandard(void* p);

template <typename T>
class nd_stack_allocator : public std::allocator<T>
{
public:
	typedef size_t size_type;
	typedef T* pointer;
	typedef const T* const_pointer;

	template <typename _Tp1>
	struct rebind
	{
		typedef nd_stack_allocator<_Tp1> other;
	};

	pointer allocate(size_type n, const void* hint = 0)
	{
		//return (pointer)malloc(n * sizeof(T));
		//fprintf(stderr, "Alloc %d bytes.\n", n * sizeof(T));
		return (pointer)::nd::internal::allocateMeehStandard(n * sizeof(T));

		//return std::allocator<T>::allocate(n, hint);
	}

	void deallocate(pointer p, size_type n)
	{
		//free(p);
		//fprintf(stderr, "Dealloc %d bytes (%p).\n", n * sizeof(T), p);
		//return std::allocator<T>::deallocate(p, n);
		::nd::internal::deallocateMeehStandard(p);
	}

	nd_stack_allocator() noexcept : std::allocator<T>()
	{
		//fprintf(stderr, "Hello allocator!\n");
	}

	nd_stack_allocator(const nd_stack_allocator& a) noexcept : std::allocator<T>(a)
	{
	}

	template <class U>
	nd_stack_allocator(const nd_stack_allocator<U>& a) noexcept : std::allocator<T>(a)
	{
	}

	~nd_stack_allocator() noexcept
	{
	}
};
}


namespace nd {
// after 2 ticks it will be destroyed
using temp_string = std::basic_string<char, std::char_traits<char>, internal::nd_stack_allocator<char>>;

// after 2 ticks it will be destroyed
template <class T>
using temp_vector = std::vector<T, internal::nd_stack_allocator<T>>;

// after 2 ticks it will be destroyed
template <class T, class _Pr = std::less<T>>
using temp_set = std::set<T, _Pr, internal::nd_stack_allocator<T>>;

// after 2 ticks it will be destroyed
template <class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
using temp_unordered_map = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq, internal::nd_stack_allocator<std::pair<
	                                              const _Kty, _Ty>>>;

template <class _Ty>
temp_string integr_to_string(_Ty val)
{
	static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
	char buf[21]; // enough for int64_t or uint64_t
	auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val);
	if (ec != std::errc()) {
		return {}; // error fallback
	}

	return temp_string(buf, ptr);
}
// to_string NARROW CONVERSIONS
[[nodiscard]] inline temp_string to_string(int _Val)
{
	// convert int to string
	return integr_to_string(_Val);
}

[[nodiscard]] inline temp_string to_string(unsigned int _Val)
{
	// convert unsigned int to string
	return integr_to_string(_Val);
}

[[nodiscard]] inline temp_string to_string(int64_t _Val)
{
	// convert long to string
	return integr_to_string(_Val);
}

[[nodiscard]] inline temp_string to_string(uint64_t _Val)
{
	// convert unsigned long to string
	return integr_to_string(_Val);
}



[[nodiscard]] inline temp_string to_string(double v)
{
	char buf[64]; // enough for any %f double with sane precision
	int n = std::snprintf(buf, sizeof(buf), "%f", v);

	if (n < 0) return temp_string();         // encode error
	if ((size_t)n < sizeof(buf))
		return temp_string(buf, n);          // fast path

	// Rare path: required length > local buffer
	temp_string out(n, '\0');
	std::snprintf(&out[0], out.size() + 1, "%f", v);
	return out;
}

[[nodiscard]] inline temp_string to_string(float _Val)
{
	// convert float to string
	return to_string(static_cast<double>(_Val));
}

[[nodiscard]] inline temp_string to_string(long double _Val)
{
	// convert long double to string
	return to_string(static_cast<double>(_Val));
}


}

inline bool operator==(const nd::temp_string& s1, const std::string& s2)
{
	return s1.length() == s2.length() &&
		std::equal(s1.begin(), s1.end(), s2.begin());
}

inline bool operator==(const std::string& s1, const nd::temp_string& s2)
{
	return operator==(s2, s1);
}

inline nd::temp_string operator+(const nd::temp_string& s1, const std::string& s2)
{
	return s1 + nd::temp_string(s2);
}

inline nd::temp_string operator+(const std::string& s1, const nd::temp_string& s2)
{
	return nd::temp_string(s1) + s2;
}
