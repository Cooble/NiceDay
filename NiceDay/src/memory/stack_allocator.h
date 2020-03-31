#pragma once
#include "ndpch.h"

void* allocateMeeh(size_t n);
void* allocateMeehStandard(size_t n);
void deallocateMeehStandard(void* p);

namespace NDUtil
{
	// See StackOverflow replies to this answer for important commentary about inheriting from std::allocator before replicating this code.
	template <typename T>
	class nd_stack_allocator : public std::allocator<T>
	{
	public:
		typedef size_t size_type;
		typedef T* pointer;
		typedef const T* const_pointer;

		template<typename _Tp1>
		struct rebind
		{
			typedef nd_stack_allocator<_Tp1> other;
		};

		pointer allocate(size_type n, const void* hint = 0)
		{
			//return (pointer)malloc(n * sizeof(T));
			//fprintf(stderr, "Alloc %d bytes.\n", n * sizeof(T));
			return (pointer)allocateMeehStandard(n * sizeof(T));

		//return std::allocator<T>::allocate(n, hint);
		}

		void deallocate(pointer p, size_type n)
		{
			//free(p);
			//fprintf(stderr, "Dealloc %d bytes (%p).\n", n * sizeof(T), p);
			//return std::allocator<T>::deallocate(p, n);
			deallocateMeehStandard(p);
		}

		nd_stack_allocator() noexcept : std::allocator<T>()
		{
			//fprintf(stderr, "Hello allocator!\n");
		}
		nd_stack_allocator(const nd_stack_allocator& a) noexcept : std::allocator<T>(a) { }
		template <class U>
		nd_stack_allocator(const nd_stack_allocator<U>& a) noexcept : std::allocator<T>(a) { }
		~nd_stack_allocator() noexcept { }
	};
}


namespace nd {
	// after 2 ticks it will be destroyed
	using temp_string = std::basic_string<char, std::char_traits<char>, NDUtil::nd_stack_allocator<char>>;

	// after 2 ticks it will be destroyed
	template <class T>
	using temp_vector = std::vector<T, NDUtil::nd_stack_allocator<T>>;

	// after 2 ticks it will be destroyed
	template <class T, class _Pr = std::less<T>>
	using temp_set = std::set<T, _Pr, NDUtil::nd_stack_allocator<T>>;

	// after 2 ticks it will be destroyed
	template <class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
	using temp_unordered_map = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq, NDUtil::nd_stack_allocator<std::pair<const _Kty, _Ty>>>;

	template <class _Ty>
	temp_string integr_to_string(const _Ty _Val) { // convert _Val to string
		static_assert(std::is_integral_v<_Ty>, "_Ty must be integral");
		using _UTy = std::make_unsigned_t<_Ty>;
		char _Buff[21]; // can hold -2^63 and 2^64 - 1, plus NUL
		char* const _Buff_end = _STD end(_Buff);
		char* _RNext = _Buff_end;
		const auto _UVal = static_cast<_UTy>(_Val);
		if (_Val < 0) {
			_RNext = std::_UIntegral_to_buff(_RNext, 0 - _UVal);
			*--_RNext = '-';
		}
		else {
			_RNext = std::_UIntegral_to_buff(_RNext, _UVal);
		}

		return temp_string(_RNext, _Buff_end);
	}

	// to_string NARROW CONVERSIONS
	_NODISCARD inline temp_string to_string(int _Val) { // convert int to string
		return nd::integr_to_string(_Val);
	}

	_NODISCARD inline temp_string to_string(unsigned int _Val) { // convert unsigned int to string
		return nd::integr_to_string(_Val);
	}

	_NODISCARD inline temp_string to_string(long _Val) { // convert long to string
		return nd::integr_to_string(_Val);
	}

	_NODISCARD inline temp_string to_string(unsigned long _Val) { // convert unsigned long to string
		return nd::integr_to_string(_Val);
	}

	_NODISCARD inline temp_string to_string(long long _Val) { // convert long long to string
		return nd::integr_to_string(_Val);
	}

	_NODISCARD inline temp_string to_string(unsigned long long _Val) { // convert unsigned long long to string
		return nd::integr_to_string(_Val);
	}

	_NODISCARD inline temp_string to_string(double _Val) { // convert double to string
		const auto _Len = static_cast<size_t>(_CSTD _scprintf("%f", _Val));
		temp_string _Str(_Len, '\0');
		_CSTD sprintf_s(&_Str[0], _Len + 1, "%f", _Val);
		return _Str;
	}

	_NODISCARD inline temp_string to_string(float _Val) { // convert float to string
		return nd::to_string(static_cast<double>(_Val));
	}

	_NODISCARD inline temp_string to_string(long double _Val) { // convert long double to string
		return nd::to_string(static_cast<double>(_Val));
	}


}

inline bool operator==(const nd::temp_string& s1,const std::string& s2)
{
	return s1.length() == s2.length() &&
		std::equal(s1.begin(), s1.end(), s2.begin());
}
inline bool operator==(const std::string& s1, const nd::temp_string& s2)
{
	return operator==(s2,s1);
}
inline nd::temp_string operator+(const nd::temp_string& s1, const std::string& s2)
{
	return s1 + nd::temp_string(s2);
}
inline nd::temp_string operator+(const std::string& s1, const nd::temp_string& s2)
{
	return nd::temp_string(s1)+s2;
}