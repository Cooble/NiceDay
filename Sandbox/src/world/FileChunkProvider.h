#pragma once
#include "ThreadedChunkProvider.h"

class FileChunkProvider:public ThreadedChunkProvider
{
private:
	std::string m_file_path;
	WorldIO::DynamicSaver m_nbt_saver;

public:
	FileChunkProvider(const std::string& file_path);
	void proccessAssignments(std::vector<WorldIOAssignment>& assignments) override;
};
