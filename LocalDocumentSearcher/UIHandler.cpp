#include "UIHandler.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "SearchRequest.h"
#include "SearchResultData.h"
#include <Windows.h>
#include <thread>

void UIHandler::Render()
{
	//ImGuiWindowFlags_NoDecoration | //ImGuiWindowFlags_NoSavedSettings
    ImGui::Begin("Local Document Search", nullptr, 0);

    ImGui::Text("My file search tool");

	ImGui::InputText("SearchQueryText", &m_SearchQuery);

	if (ImGui::Button("Search") && !m_SearchQuery.empty())
	{
		m_SearchResults.clear();
		SearchRequestData requestData;
		requestData.TextToSearch = std::wstring(m_SearchQuery.begin(), m_SearchQuery.end());
		requestData.DirectoryToSearch = L"\\\\?\\C:\\";
		requestData.SearchFileName = true;
		requestData.SearchFolderName = true;
		requestData.SearchFileContent = false;
		requestData.CaseSensitive = true;
		requestData.LookForExactMatch = false;

		m_CurrentSearchRequest = std::make_unique<SearchRequest>(requestData);
		std::thread searchThread(&SearchRequest::ExecuteSearchRequest, m_CurrentSearchRequest.get());
		searchThread.detach();
	}

	std::vector<SearchResultData> results;
	// Braces so the lock goes out off scope and unlocks
	{
		std::lock_guard<std::mutex> lock(m_SearchResultsMutex);
		results = m_SearchResults;
	}

	for (const SearchResultData& result : results)
	{
		ImGui::PushID(&result);
		if (ImGui::Button("Reveal"))
		{
			RevealFile(result.FilePath.wstring());
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::Text(result.IsDirectory ? "Folder: %s" :"File: %s", result.FileName.string().c_str());
		ImGui::SameLine();
		ImGui::Text("Size: %llu bytes", result.FileSizeInBytes);
		ImGui::SameLine();
		ImGui::Text("Path: %s", result.FilePath.string().c_str());
	}

    ImGui::End();
}

void UIHandler::AddSearchResult(const SearchResultData& result)
{
	std::lock_guard<std::mutex> lock(m_SearchResultsMutex);
	m_SearchResults.emplace_back(result);
}

void UIHandler::OpenFolder(const std::wstring& path)
{
	ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void UIHandler::RevealFile(const std::wstring& path)
{
	const std::wstring args = L"/select,\"" + path + L"\"";

	ShellExecuteW(nullptr, L"open", L"explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL);
}
