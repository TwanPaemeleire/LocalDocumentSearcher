#include "ProjectRunner.h"
#include <chrono>
#include <thread>
#include "Renderer.h"
#include <SDL3/SDL.h>
#include "imgui_impl_sdl3.h"

void ProjectRunner::Run()
{
	auto lastTime = std::chrono::high_resolution_clock::now();
	float lag = 0.f;

	Renderer::GetInstance().Init();

	while (m_ShouldContinue)
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT) m_ShouldContinue = false;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) m_ShouldContinue = false;
		}

		Renderer::GetInstance().Render();

		const auto sleepTime = std::chrono::milliseconds(m_MsPerFrame) - (std::chrono::high_resolution_clock::now() - currentTime);
		std::this_thread::sleep_for(sleepTime);
	}
	Renderer::GetInstance().Cleanup();
}

void ProjectRunner::RequestStop()
{
	m_ShouldContinue = false;
}

