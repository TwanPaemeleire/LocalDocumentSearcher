#pragma once
class ProjectRunner
{
public:
	void Run();
	void RequestStop();

private:
	bool m_ShouldContinue = true;
	float m_FixedTimeStep = 1.0f / 60.0f;
	int m_MsPerFrame = 1000 / 60;
};