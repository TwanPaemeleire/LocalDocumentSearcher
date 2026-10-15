#pragma once
#include <string>
#include <filesystem>

struct SearchRequestData
{
	std::wstring TextToSearch;
	std::wstring ExtensionToFilter;
	std::filesystem::path DirectoryToSearch;
	bool SearchFileName = true;
	bool SearchFolderName = false;
	bool SearchFileContent = false;
	bool CaseSensitive = false;
	bool LookForExactMatch = false;
};