#include "UIHandler.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "SearchRequest.h"
#include "SearchResultData.h"
#include <Windows.h>
#include <thread>
#include "Renderer.h"
#include "imgui_internal.h"
#include "ProjectRunner.h"
#include <shobjidl.h>

void UIHandler::Render()
{
    SetDarkPastelImGuiStyle();
	//ImGuiWindowFlags_NoDecoration | //ImGuiWindowFlags_NoSavedSettings
	ImGui::SetNextWindowPos(ImVec2(0, 0), 0);
	ImGui::SetNextWindowSize(Renderer::GetInstance().GetWindowSize(), 0);
    ImGui::Begin("Local Document Search", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

	ImGui::InputText("SearchQueryText", &m_TextToSearchForHolder);
	m_CurrentSearchRequestData.TextToSearch = std::wstring(m_TextToSearchForHolder.begin(), m_TextToSearchForHolder.end());

    ImGuiID close_button_id = ImGui::GetID("##close_button");
    if (ImGui::CloseButton(close_button_id, ImVec2(ImGui::GetWindowSize().x - 15, 5)))
    {
        ProjectRunner::GetInstance().RequestStop();
    }

	if (ImGui::Button("Search") && !m_CurrentSearchRequestData.DirectoryToSearch.empty())
	{
		m_SearchResults.clear();
		m_CurrentSearchRequest = std::make_unique<SearchRequest>(m_CurrentSearchRequestData);
		std::thread searchThread(&SearchRequest::ExecuteSearchRequest, m_CurrentSearchRequest.get());
		searchThread.detach();
	}

    if (ImGui::Button("ChooseBasePath"))
    {
        // User gets windows popup anc can choose the root folder
        m_CurrentSearchRequestData.DirectoryToSearch = L"\\\\?\\" + SelectFolder() + L"\\";
    }
	ImGui::SameLine();
	ImGui::Text("BasePath: %s", m_CurrentSearchRequestData.DirectoryToSearch.string().c_str());

	ImGui::InputText("ExtensionsToFilter", &m_ExtensionsToFilterHolder);
	m_CurrentSearchRequestData.ExtensionToFilter = std::wstring(m_ExtensionsToFilterHolder.begin(), m_ExtensionsToFilterHolder.end());

    if (ImGui::BeginCombo("SearchOptions", "Search Options"))
    {
        ImGui::Checkbox("SearchFileName", &m_CurrentSearchRequestData.SearchFileName);
        ImGui::Checkbox("SearchFolderName", &m_CurrentSearchRequestData.SearchFolderName);
        ImGui::Checkbox("SearchFileContent", &m_CurrentSearchRequestData.SearchFileContent);
        ImGui::Checkbox("CaseSensitive", &m_CurrentSearchRequestData.CaseSensitive);
        ImGui::Checkbox("LookForExactMatch", &m_CurrentSearchRequestData.LookForExactMatch);
        ImGui::Checkbox("IgnoreRecyclingBin", &m_CurrentSearchRequestData.IgnoreRecyclingBin);

        ImGui::EndCombo();
    }

	std::vector<SearchResultData> results;
	// Braces so the lock goes out off scope and unlocks
	{
		std::lock_guard<std::mutex> lock(m_SearchResultsMutex);
		results = m_SearchResults;
	}

	ImGui::BeginListBox("SearchResults", ImVec2(Renderer::GetInstance().GetWindowSize().x - 100, 400));

    for (int i = 0; i < results.size(); ++i)
    {
        const SearchResultData& result = results[i];

		ImGui::PushID(i);
		if (ImGui::Button("Reveal"))
		{
			RevealFile(result.FilePath.wstring());
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::PushID(-i);
		if (ImGui::Selectable(result.FileName.string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
		{
			if (ImGui::IsMouseDoubleClicked(0))
			{
				RevealFile(result.FilePath.wstring());
			}
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::Text(result.IsDirectory ? "Folder: %s" :"File: %s", result.FileName.string().c_str());
		ImGui::SameLine();
		ImGui::Text("Size: %llu bytes", result.FileSizeInBytes);
		ImGui::SameLine();
		ImGui::Text("Path: %s", result.FilePath.string().c_str());
	}
	ImGui::EndListBox();
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

std::wstring UIHandler::SelectFolder()
{
    HWND owner = nullptr;
    std::wstring result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool comInitialized = SUCCEEDED(hr);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) return result;

    IFileOpenDialog* dialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(hr))
    {
        // Keep the default options and add "pick folders"
        DWORD options = 0;
        dialog->GetOptions(&options);
        dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        dialog->SetTitle(L"Select a folder");

        hr = dialog->Show(owner);
        if (SUCCEEDED(hr))
        {
            IShellItem* item = nullptr;
            if (SUCCEEDED(dialog->GetResult(&item)))
            {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                {
                    result = path;
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }
        dialog->Release();
    }
    if (comInitialized) CoUninitialize();

    return result;
}

void UIHandler::SetDarkPastelImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Backgrounds 
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f); // Dark grey base
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.33f, 0.42f, 0.40f);

    //Text 
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.65f, 0.70f, 1.00f);

    // Headers 
    colors[ImGuiCol_Header] = ImVec4(0.36f, 0.42f, 0.55f, 0.60f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.50f, 0.68f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.46f, 0.55f, 0.75f, 1.00f);

    // Buttons 
    colors[ImGuiCol_Button] = ImVec4(0.28f, 0.34f, 0.48f, 0.70f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.36f, 0.45f, 0.65f, 0.85f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.50f, 0.70f, 1.00f);

    // Frames 
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.38f, 0.50f, 1.00f);

    // Tabs 
    colors[ImGuiCol_Tab] = ImVec4(0.26f, 0.30f, 0.42f, 0.80f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.36f, 0.42f, 0.58f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.50f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.24f, 0.32f, 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.30f, 0.36f, 0.50f, 1.00f);

    // Titles 
    colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.12f, 0.15f, 0.75f);

    // Scrollbars 
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.30f, 0.38f, 0.60f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.40f, 0.50f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.50f, 0.65f, 1.00f);

    // Checkboxes / Radios 
    colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.85f, 1.00f, 1.00f);

    // Sliders 
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.75f, 1.00f, 1.00f);

    // Resize Grip 
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.40f, 0.50f, 0.60f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.50f, 0.60f, 0.80f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.60f, 0.80f, 1.00f);

    // Separator 
    colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.40f, 0.48f, 0.7f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.60f, 0.72f, 0.9f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.65f, 0.70f, 0.85f, 1.0f);

    // Menus and Tooltips 
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.15f, 0.17f, 1.00f);
    // colors[ImGuiCol_TooltipBg]          = ImVec4(0.18f, 0.20f, 0.25f, 0.95f);

    // Drag & Drop 
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.50f, 0.85f, 1.00f, 0.90f);

    // Style Metrics 
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 5.0f;

    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(16, 16);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 10);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 20.0f;
}