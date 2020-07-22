#pragma once
#include "ndpch.h"

typedef std::string Stringo;

namespace SUtil
{
	inline bool startsWith(const Stringo& s, const Stringo& prefix)
	{
		return strncmp(prefix.c_str(), s.c_str(), prefix.size()) == 0;
	}

	inline bool startsWith(const Stringo& s, char prefix)
	{
		return !s.empty() && s[0] == prefix;
	}

	inline bool startsWith(const char* s, const char* prefix)
	{
		return strncmp(s, prefix, strlen(prefix)) == 0;
	}

	inline bool startsWith(const char* s, char prefix)
	{
		return strlen(s) && s[0] == prefix;
	}

	inline bool endsWith(const Stringo& s, const Stringo& suffix)
	{
		return suffix.size() <= s.size() && strncmp(suffix.c_str(), s.c_str() + s.size() - suffix.size(), suffix.size())
			== 0;
	}

	inline bool endsWith(const Stringo& s, char suffix)
	{
		return !s.empty() && s[s.size() - 1] == suffix;
	}

	inline bool endsWith(const char* s, const char* suffix)
	{
		auto sizeS = strlen(s);
		auto sizeSuf = strlen(suffix);

		return sizeSuf <= sizeS && strncmp(suffix, s + sizeS - sizeSuf, sizeSuf) == 0;
	}

	inline bool endsWith(const char* s, char suffix)
	{
		auto sizeS = strlen(s);

		return sizeS && s[sizeS - 1] == suffix;
	}

	inline bool replaceWith(Stringo& src, char what, char with)
	{
		for (int i = 0; i < src.size(); ++i)
		{
			auto& id = src.data()[i];
			bool isWhat = id == what;
			id = isWhat * with + src[i] * (!isWhat);
		}
		return true;
	}

	inline bool replaceWith(Stringo& src, const char* what, const char* with)
	{
		Stringo out;
		size_t whatlen = strlen(what);
		out.reserve(src.size());
		size_t ind = 0;
		size_t lastInd = 0;
		while (true)
		{
			ind = src.find(what, ind);
			if (ind == Stringo::npos)
			{
				out += src.substr(lastInd);
				break;
			}
			out += src.substr(lastInd, ind - lastInd) + with;
			ind += whatlen;
			lastInd = ind;
		}
		src = out;
		return true;
	}


	inline bool replaceWith(Stringo& src, const char* what, const char* with, int times)
	{
		for (int i = 0; i < times; ++i)
			replaceWith(src, what, with);
		return true;
	}

	//populates words with words that were crated by splitting line with one char of dividers
	//NOTE!: when using views
	//	if you delete the line (or src of the line)
	//		-> string_views will no longer be valid!!!!!
	/*template <typename StringOrView>
	void splitString(const std::string_view& line, std::vector<StringOrView>& words, const char* divider = " \n\t")
	{
		auto beginIdx = line.find_first_not_of(divider, 0);
		while (beginIdx != Stringo::npos)
		{
			auto endIdx = line.find_first_of(divider, beginIdx);
			if (endIdx != Stringo::npos)
			{
				words.emplace_back(line.begin() + beginIdx, endIdx - beginIdx);
				beginIdx = line.find_first_not_of(divider, endIdx);
			}
			else {
				words.emplace_back(line.begin() + beginIdx, line.size() - beginIdx);
				break;
			}
		}
	}*/
	inline void splitString(const Stringo& line, std::vector<Stringo>& words, const char* divider = " \n\t")
	{
		auto beginIdx = line.find_first_not_of(divider, 0);
		while (beginIdx != Stringo::npos)
		{
			auto endIdx = line.find_first_of(divider, beginIdx);
			if (endIdx != Stringo::npos)
			{
				words.emplace_back(line.data() + beginIdx, endIdx - beginIdx);
				beginIdx = line.find_first_not_of(divider, endIdx);
			}
			else
			{
				words.emplace_back(line.data() + beginIdx, line.size() - beginIdx);
				break;
			}
		}
	}

	/*inline void splitString(const std::string_view& line, std::vector<std::string_view>& words, const char* divider = " \n\t")
	{
		auto beginIdx = line.find_first_not_of(divider, 0);
		while (beginIdx != Stringo::npos)
		{
			auto endIdx = line.find_first_of(divider, beginIdx);
			if (endIdx != Stringo::npos)
			{
				words.emplace_back(line.begin() + beginIdx, endIdx - beginIdx);
				beginIdx = line.find_first_not_of(divider, endIdx);
			}
			else {
				words.emplace_back(line.begin() + beginIdx, line.size() - beginIdx);
				break;
			}
		}
	}*/

	// iterates over a string_view divided by "divider"
	// Example:
	//		if(!ignoreBlanks)
	//			"\n\nBoo\n" -> "","","Boo",""
	//		else
	//			"\n\nBoo\n" -> "Boo"
	template <bool ignoreBlanks = false>
	class SplitIterator
	{
	private:
		std::string_view m_source;
		std::string_view m_pointer;
		const char* m_divider;
		// this fixes 1 edge case when m_source end with m_divider
		bool m_ending = false;


		void step()
		{
			if (ignoreBlanks)
			{
				bool first = true;
				while (m_pointer.empty() || first)
				{
					first = false;

					if (!operator bool())
						return;
					//check if this is ending
					//the next one will be false
					if (m_source.size() == m_pointer.size())
					{
						m_source = std::string_view(nullptr, 0);
					}
					else
					{
						auto viewSize = m_pointer.size();
						m_source = std::string_view(m_source.data() + viewSize + 1, m_source.size() - viewSize - 1);
						//shift source by viewSize

						auto nextDivi = m_source.find_first_of(m_divider, 0);
						if (nextDivi != Stringo::npos)
							m_pointer = std::string_view(m_source.data(), nextDivi);
						else
							m_pointer = std::string_view(m_source.data(), m_source.size());
					}
				}
			}
			else
			{
				if (!operator bool())
					return;
				//check if this is ending
				//the next one will be false
				if (m_source.size() == m_pointer.size())
				{
					m_source = std::string_view(nullptr, 0);
					m_ending = false;
				}
				else
				{
					auto viewSize = m_pointer.size();
					m_source = std::string_view(m_source.data() + viewSize + 1, m_source.size() - viewSize - 1);
					//shift source by viewSize
					if (m_source.empty())
						m_ending = true;
					auto nextDivi = m_source.find_first_of(m_divider, 0);
					if (nextDivi != Stringo::npos)
						m_pointer = std::string_view(m_source.data(), nextDivi);
					else
						m_pointer = std::string_view(m_source.data(), m_source.size());
				}
			}
		}

	public:
		SplitIterator(std::string_view src, const char* divider)
			: m_source(src), m_divider(divider)
		{
			auto div = m_source.find_first_of(m_divider, 0);
			m_pointer = std::string_view(m_source.data(), div == Stringo::npos ? m_source.size() : div);
			if (ignoreBlanks)
			{
				if (m_pointer.empty())
					step();
			}
		}

		SplitIterator(const SplitIterator& s) = default;

		operator bool() const
		{
			return !m_source.empty() || m_ending;
		}

		SplitIterator& operator+=(size_t delta)
		{
			while (this->operator bool() && delta)
			{
				delta--;
				step();
			}
			return (*this);
		}

		SplitIterator& operator++()
		{
			step();
			return (*this);
		}

		SplitIterator operator++(int)
		{
			auto temp(*this);
			step();
			return temp;
		}

		const std::string_view& operator*() { return m_pointer; }
		const std::string_view* operator->() { return &m_pointer; }
	};
}
