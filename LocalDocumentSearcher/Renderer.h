#pragma once
#include "Singleton.h"

struct SDL_Renderer;
struct SDL_Window;

class Renderer final : public Singleton<Renderer>
{
public:
	void Init();
	void Render();
	void Cleanup();

private:
	SDL_Renderer* m_Renderer;
	SDL_Window* m_Window;
};