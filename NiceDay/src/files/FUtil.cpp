#include "FUtil.h"
#include <codecvt>
//current implementation works only on windows

static std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}
void FUtil::cleanPathString(std::string& s)
{
	SUtil::replaceWith(s, '\\', '/');
	SUtil::replaceWith(s, "//", "/", 2);
}

std::string FUtil::readFileString(std::string_view path)
{
   auto p = ND_RESLOC(path);
   if (!exists(p))
	  return "";
   const std::ifstream input_stream(p, std::ios_base::binary);

   if (input_stream.fail())
   {
	  ND_ERROR("Failed to open file {}", p);
	  return "";
   }

   std::stringstream buffer;
   buffer << input_stream.rdbuf();
   return buffer.str();
}


std::string FUtil::getAbsolutePath(const char* fileName)
{
	std::string out = getExecutableFolderPath() + fileName;
	return std::filesystem::exists(out) ? out : "";
}

const std::string& FUtil::getExecutableFolderPath()
{
	static std::string out;
	if (out.empty()) {
		out = getExecutablePath().substr(0, getExecutablePath().find_last_of('/') + 1);
	}
	return out;
}

const std::string& FUtil::getExecutablePath()
{
	static std::string out;
	if (out.empty()) {
		WCHAR path[260];
		GetModuleFileNameW(NULL, path, 260);
		out = ws2s(std::wstring(path));
		cleanPathString(out);
	}
	return out;
}

std::vector<std::string> FUtil::fileList(std::string_view folder_path, FileSearchFlags flags)
{
	std::vector<std::string> out;
	std::filesystem::file_time_type bestTime;
	bool nope = flags & (FileSearchFlags_Oldest | FileSearchFlags_Newest);
	if (!FUtil::exists(folder_path))
		return out;

	if (!(flags & FileSearchFlags_Recursive)) {
		for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
			std::string s = ws2s(std::wstring(entry.path().c_str()));
			if (entry.is_directory() && flags & FileSearchFlags_OnlyFiles)
			{
			}
			else if (entry.is_regular_file() && flags & FileSearchFlags_OnlyDirectories)
			{
			}
			else {
				if (nope)
				{
					nope = false;
					bestTime = last_write_time(entry.path());
					out.push_back(s);
				}
				auto currentTime = last_write_time(entry.path());

				if (
					((flags & FileSearchFlags_Newest) && currentTime > bestTime)
					|| ((flags & FileSearchFlags_Oldest) && currentTime < bestTime))
				{
					bestTime = currentTime;
					out.clear();
					out.push_back(s);
				}
				else if (!(flags & (FileSearchFlags_Oldest | FileSearchFlags_Newest)))
					out.push_back(s);
			}

		}
	}
	else
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(folder_path)) {
			std::string s = ws2s(std::wstring(entry.path().c_str()));
			if (entry.is_directory() && flags & FileSearchFlags_OnlyFiles)
			{
			}
			else if (entry.is_regular_file() && flags & FileSearchFlags_OnlyDirectories)
			{
			}
			else {
				if (nope)
				{
					nope = false;
					bestTime = last_write_time(entry.path());
					out.push_back(s);
				}
				auto currentTime = last_write_time(entry.path());

				if (
					((flags & FileSearchFlags_Newest) && currentTime > bestTime)
					|| ((flags & FileSearchFlags_Oldest) && currentTime < bestTime))
				{
					bestTime = currentTime;
					out.clear();
					out.push_back(s);
				}
				else if (!(flags & (FileSearchFlags_Oldest | FileSearchFlags_Newest)))
					out.push_back(s);
			}

		}
	}
	return out;

}

