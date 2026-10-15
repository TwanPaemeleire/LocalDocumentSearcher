#include "ProjectRunner.h"
#include <chrono>
#include <thread>
#include "Renderer.h"

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

