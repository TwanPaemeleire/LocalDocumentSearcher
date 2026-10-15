#pragma once
#include <string>
#include <filesystem>

struct SearchResultData
{
	std::filesystem::path FileName;
	std::filesystem::path FilePath;
	uintmax_t FileSizeInBytes = 0;
	bool IsDirectory = false;
};