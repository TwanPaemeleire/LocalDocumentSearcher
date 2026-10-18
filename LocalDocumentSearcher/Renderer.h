#pragma once
#include "Singleton.h"

struct SDL_Renderer;
struct SDL_Window;
struct ImVec2;

class Renderer final : public Singleton<Renderer>
{
public:
	void Init();
	void Render();
	void Cleanup();
	ImVec2 GetWindowSize() const;

private:
	SDL_Renderer* m_Renderer;
	SDL_Window* m_Window;
};