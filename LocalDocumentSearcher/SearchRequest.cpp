#include "SearchRequest.h"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include "SearchResultData.h"

SearchRequest::SearchRequest(const SearchRequestData& data)
	:m_Data(data)
{
    if (!m_Data.CaseSensitive)
    {
        std::transform(m_Data.TextToSearch.begin(), m_Data.TextToSearch.end(), m_Data.TextToSearch.begin(), ::tolower);
    }
}

void SearchRequest::ExecuteSearchRequest()
{
    std::vector<SearchResultData> searchResults;
    std::error_code errorCode;

    std::filesystem::recursive_directory_iterator it(m_Data.DirectoryToSearch, std::filesystem::directory_options::skip_permission_denied, errorCode);
    std::filesystem::recursive_directory_iterator end;

    while (it != end)
    {
        const auto& entry = *it;
        std::error_code typeError;
        const bool isDirectory = entry.is_directory(typeError);

        const std::filesystem::path& path = entry.path();
        std::wstring filename = path.filename().wstring();
        if(!m_Data.CaseSensitive) std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
            
        bool meetsSearchConditions = false;

        bool nameIsMatch = (m_Data.LookForExactMatch) ? filename == m_Data.TextToSearch : filename.find(m_Data.TextToSearch) != std::wstring::npos;

        if (isDirectory && m_Data.SearchFolderName && nameIsMatch)
        {
            meetsSearchConditions = true;
        }
        if (!isDirectory && m_Data.SearchFileName && nameIsMatch)
        {
            meetsSearchConditions = true;
        }
        if (!meetsSearchConditions && m_Data.SearchFileContent)
        {
            // Search inside file, keep m_Data.LookForExactMatch in account
        }

        if (meetsSearchConditions)
        {
            SearchResultData resultData;
            resultData.FileName = entry.path().filename();
            resultData.FilePath = entry.path();
            resultData.FileSizeInBytes = entry.file_size();
            resultData.IsDirectory = isDirectory;
            searchResults.emplace_back(resultData);

            std::string typeName = (isDirectory) ? "Folder " : "File ";
            std::cout << "Found " << typeName;
            std::cout << "| Name: " << resultData.FileName;
            std::cout << " | Size: " << resultData.FileSizeInBytes;
            std::cout << " | Path: " << resultData.FilePath << std::endl;
        }

        // If checking this entry's type failed, don't recurse further into it
        if (typeError)
        {
            it.disable_recursion_pending();
            std::cerr << "Type error: " << path.string() << " | " << typeError.message() << '\n';
        }
        errorCode.clear();
        it.increment(errorCode);
        if (errorCode) // Check if there was an error during the incrementation
        {
            std::cerr << "Traversal error: " << errorCode.message() << '\n';
        }
    }
    std::cout << "Finished!" << std::endl;
}

void SearchRequest::StopSearchRequest()
{
}