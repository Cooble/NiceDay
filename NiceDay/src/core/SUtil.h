#pragma once
#include "ndpch.h"





namespace SUtil
{
   typedef std::string Stringo;
   typedef std::string_view Viewo;

   inline bool equals(const char* a, const char* c)
   {
	  return strcmp(a, c) == 0;
   }

   inline bool startsWith(Viewo s, Viewo prefix)
   {
	  return prefix.size() <= s.size() && (strncmp(prefix.data(), s.data(), prefix.size()) == 0);
   }
   inline bool startsWith(Viewo s, char prefix)
   {
	  return !s.empty() && s[0] == prefix;
   }
   inline bool startsWith(const char* s, const char* prefix)
   {
	  return strncmp(s, prefix, strlen(prefix)) == 0;
   }


   inline bool endsWith(Viewo s, Viewo suffix)
   {
	  return suffix.size() <= s.size() && strncmp(suffix.data(), s.data() + s.size() - suffix.size(), suffix.size()) == 0;
   }
   inline bool endsWith(Viewo s, char suffix)
   {
	  return !s.empty() && s[s.size() - 1] == suffix;
   }
   inline bool endsWith(const char* s, const char* suffix)
   {
	  auto sizeS = strlen(s);
	  auto sizeSuf = strlen(suffix);

	  return sizeSuf <= sizeS && strncmp(suffix, s + sizeS - sizeSuf, sizeSuf) == 0;
   }
   inline void toLower(char* s)
   {
	  int l = strlen(s);
	  int ind = 0;
	  // spec of "simd"
	  for (int i = 0; i < l / 4; i++) {
		 s[ind] = std::tolower(s[ind]);
		 s[ind + 1] = std::tolower(s[ind + 1]);
		 s[ind + 2] = std::tolower(s[ind + 2]);
		 s[ind + 3] = std::tolower(s[ind + 3]);
		 ind += 4;
	  }
	  //do the rest linearly
	  for (int i = 0; i < (l & 3); ++i)
	  {
		 s[ind++] = std::tolower(s[ind]);
	  }
   }
   inline void toLower(Stringo& ss)
   {
	  int l = ss.size();
	  auto s = ss.data();
	  int ind = 0;
	  // spec of "simd"
	  for (int i = 0; i < l / 4; i++) {
		 s[ind] = std::tolower(s[ind]);
		 s[ind + 1] = std::tolower(s[ind + 1]);
		 s[ind + 2] = std::tolower(s[ind + 2]);
		 s[ind + 3] = std::tolower(s[ind + 3]);
		 ind += 4;
	  }
	  //do the rest linearly
	  for (int i = 0; i < (l & 3); ++i)
		 s[ind++] = std::tolower(s[ind]);
   }
   inline void toUpper(char* s)
   {
	  int l = strlen(s);
	  int ind = 0;
	  // spec of "simd"
	  for (int i = 0; i < l / 4; i++) {
		 s[ind] = std::toupper(s[ind]);
		 s[ind + 1] = std::toupper(s[ind + 1]);
		 s[ind + 2] = std::toupper(s[ind + 2]);
		 s[ind + 3] = std::toupper(s[ind + 3]);
		 ind += 4;
	  }
	  //do the rest linearly
	  for (int i = 0; i < (l & 3); ++i)
	  {
		 s[ind++] = std::toupper(s[ind]);
	  }
   }
   inline void toUpper(Stringo& ss)
   {
	  int l = ss.size();
	  auto s = ss.data();
	  int ind = 0;
	  // spec of "simd"
	  for (int i = 0; i < l / 4; i++) {
		 s[ind] = std::toupper(s[ind]);
		 s[ind + 1] = std::toupper(s[ind + 1]);
		 s[ind + 2] = std::toupper(s[ind + 2]);
		 s[ind + 3] = std::toupper(s[ind + 3]);
		 ind += 4;
	  }
	  //do the rest linearly
	  for (int i = 0; i < (l & 3); ++i)
		 s[ind++] = std::toupper(s[ind]);
   }

   inline bool replaceWith(char* src, char what, char with)
   {
	  for (int i = 0; true; ++i)
	  {
		 auto& id = src[i];
		 if (id == '\0') return true;
		 bool isWhat = id == what;
		 id = isWhat * with + src[i] * (!isWhat);
	  }
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
   void splitString(const Viewo& line, std::vector<StringOrView>& words, const char* divider = " \n\t")
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


   /*inline void splitString(const Viewo& line, std::vector<Viewo>& words, const char* divider = " \n\t")
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
   // iterates over a string_view divided by "divider" (any chars in const char*)
   // Example:
   //		if(!ignoreBlanks)
   //			"\n\nBoo\n" -> "","","Boo",""
   //		else
   //			"\n\nBoo\n" -> "Boo"
   //
   //
   //	enable throwException to throw Stringo("No Word Left") 
   //	if(operator bool()==false)
   //		-> on calling operator*() or operator->()
   //
   template <bool ignoreBlanks = false, typename DividerType = const char*, bool throwException = false>
   class SplitIterator
   {
	  static_assert(std::is_same<DividerType, const char*>::value || std::is_same<DividerType, char>::value);
   private:
	  Viewo m_source;
	  Viewo m_pointer;
	  // this fixes 1 edge case when m_source end with m_divider
	  DividerType m_divider;
	  bool m_ending = false;


	  void step()
	  {
		 if (!operator bool())
			return;

		 if constexpr (ignoreBlanks)
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
				  m_source = Viewo(nullptr, 0);
			   }
			   else
			   {
				  auto viewSize = m_pointer.size();
				  m_source = Viewo(m_source.data() + viewSize + 1, m_source.size() - viewSize - 1);
				  //shift source by viewSize

				  auto nextDivi = m_source.find_first_of(m_divider, 0);
				  if (nextDivi != Stringo::npos)
					 m_pointer = Viewo(m_source.data(), nextDivi);
				  else
					 m_pointer = Viewo(m_source.data(), m_source.size());
			   }
			}
		 }
		 else
		 {
			//check if this is ending
			//the next one will be false
			if (m_source.size() == m_pointer.size())
			{
			   m_source = Viewo(nullptr, 0);
			   m_ending = false;
			}
			else
			{
			   auto viewSize = m_pointer.size();
			   m_source = Viewo(m_source.data() + viewSize + 1, m_source.size() - viewSize - 1);
			   //shift source by viewSize
			   if (m_source.empty())
				  m_ending = true;
			   auto nextDivi = m_source.find_first_of(m_divider, 0);
			   if (nextDivi != Stringo::npos)
				  m_pointer = Viewo(m_source.data(), nextDivi);
			   else
				  m_pointer = Viewo(m_source.data(), m_source.size());
			}
		 }
	  }

   public:
	  SplitIterator(Viewo src, DividerType divider)
		 : m_source(src), m_divider(divider)
	  {
		 auto div = m_source.find_first_of(m_divider, 0);
		 m_pointer = Viewo(m_source.data(), div == Stringo::npos ? m_source.size() : div);
		 if constexpr (ignoreBlanks)
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
		 if (this->operator bool())
			step();
		 return (*this);
	  }

	  SplitIterator operator++(int)
	  {
		 auto temp(*this);
		 step();
		 return temp;
	  }

	  const Viewo& operator*()
	  {
		 if (throwException && !operator bool()) //Attempt to access* it or it->when no word is left in the iterator
			throw Stringo("No Word Left");
		 return m_pointer;
	  }

	  const Viewo* operator->()
	  {
		 if (throwException && !operator bool()) //Attempt to access *it or it-> when no word is left in the iterator
			throw Stringo("No Word Left");
		 return &m_pointer;
	  }
   };

   //removes comments: (replaces with ' ')
   //	1. one liner starting with "//"
   //	2. block comment bounded by "/*" and "*/"
   inline void removeComments(Stringo& src)
   {
	  size_t offset = 0;
	  bool opened = false; //multiliner opened
	  size_t openedStart = 0;
	  while (true) {
		 auto slash = src.find_first_of('/', offset);
		 if (slash != Stringo::npos)
		 {
			Stringo s = src.substr(slash);
			if (!opened)
			{
			   if (src.size() == slash - 1)
				  return;

			   char next = src[slash + 1];
			   if (next == '/') //one liner
			   {
				  auto end = src.find_first_of('\n', slash + 1);
				  if (end == Stringo::npos)
				  {
					 memset(src.data() + slash, ' ', src.size() - 1 - slash);
					 return;
				  }
				  memset(src.data() + slash, ' ', end - slash);
				  offset = end;
			   }
			   else if (next == '*')
			   {
				  opened = true;
				  offset = slash + 1;
				  openedStart = slash;
			   }
			   else offset = slash + 1;
			}
			else {
			   if (src[slash - 1] == '*') {
				  opened = false;
				  memset(src.data() + openedStart, ' ', slash - openedStart);
				  offset = slash + 1;
			   }
			   offset = slash + 1;
			}
		 }
		 else if (opened) {
			memset(src.data() + openedStart, ' ', src.size() - 1 - openedStart);
			return;
		 }
		 else return;
	  }
   }
   // converts utf8 encoded string to zero terminated int array of codePoints
   // transfers ownership of returned array (don't forget free())
   // length will be set to size returned array (excluding zero terminator)
   const int* utf8toCodePointsArray(const char* c, int* length = nullptr);

   std::u32string utf8toCodePoints(const char* c);

   inline std::u32string utf8toCodePoints(const Stringo& c) { return utf8toCodePoints(c.c_str()); }

   // converts ascii u32 string to string
   // use only if you know that there are only ascii characters
   Stringo u32StringToString(std::u32string_view s);


   // returns first occurrence of digit or nullptr
   inline const char* skipToNextDigit(const char* c)
   {
	  c--;
	  while (*(++c)) {
		 if (*c >= '0' && *c <= '9')
			return c;
	  }
	  return nullptr;
   }

   // keeps parsing numbers until size is reached or until there are no numbers
   // actualSize is set to number of numbers actually parsed
   template <int size, typename numberType>
   void parseNumbers(const char* c, numberType ray[size], int* actualSize = nullptr)
   {
	  size_t chars = 0;

	  for (int i = 0; i < size; ++i)
	  {
		 if ((c = skipToNextDigit(c + chars)) != nullptr)
		 {
			if (std::is_same<numberType, int>::value)
			   ray[i] = std::stoi(c, &chars);
			else  if (std::is_same<numberType, float>::value)
			   ray[i] = std::stof(c, &chars);
			else  if (std::is_same<numberType, double>::value)
			   ray[i] = std::stod(c, &chars);
			else static_assert("invalid type");
		 }
		 else {
			if (actualSize) *actualSize = i;
			return;
		 }
	  }
	  if (actualSize) *actualSize = size;
   }


   // keeps parsing numbers until size is reached or until there are no numbers
   // actualSize is set to number of numbers actually parsed
   template <int size, typename numberType>
   void parseNumbers(const Stringo& s, numberType ray[size], int* actualSize = nullptr)
   {
	  parseNumbers<size>(s.c_str(), ray, actualSize);
   }

}


