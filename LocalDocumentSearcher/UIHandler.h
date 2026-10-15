#pragma once
#include "Singleton.h"
#include <string>
#include <vector>
#include "SearchResultData.h"
#include "SearchRequest.h"
#include <memory>
#include <mutex>

class UIHandler final : public Singleton<UIHandler>
{
public:
	void Render();
	void AddSearchResult(const SearchResultData& result);
	void OpenFolder(const std::wstring& path);
	void RevealFile(const std::wstring& path);

private:
	std::string m_SearchQuery;
	std::mutex m_SearchResultsMutex;
	std::vector<SearchResultData> m_SearchResults;
	std::unique_ptr<SearchRequest> m_CurrentSearchRequest = nullptr;
};