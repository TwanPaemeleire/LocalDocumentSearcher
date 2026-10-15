#include <string>
#include <Windows.h>
#include "SearchRequest.h"
#include "ProjectRunner.h"


void OpenFolder(const std::wstring& path)
{
    ShellExecuteW(
        nullptr, L"open", path.c_str(),
        nullptr, nullptr, SW_SHOWNORMAL
    );
}

void RevealFile(const std::wstring& path)
{
    const std::wstring args = L"/select,\"" + path + L"\"";

    ShellExecuteW(
        nullptr, L"open", L"explorer.exe",
        args.c_str(), nullptr, SW_SHOWNORMAL
    );
}

int main(int argc, char* argv[])
{
	ProjectRunner runner;
    runner.Run();
    //SearchRequestData requestData;
    //requestData.TextToSearch = L"test";
    //requestData.DirectoryToSearch = L"\\\\?\\C:\\C++Projects\\";
    //requestData.LookForExactMatch = false;
    //requestData.SearchFileName = true;
    //requestData.SearchFileContent = false;
    //requestData.SearchFolderName = true;
    //requestData.CaseSensitive = false;
    //
    //SearchRequest request = SearchRequest(requestData);
    //request.ExecuteSearchRequest();
    // 
    // 
    // std::cout << "Which index file do you want to open: ";
    // int indexToOpen = 0;
	// std::cin >> indexToOpen;
    // RevealFile(results[indexToOpen].wstring());

	return 0;
}