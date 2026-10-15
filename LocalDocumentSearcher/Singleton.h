#pragma once

template <typename Type>
class Singleton
{
public:
	static Type& GetInstance()
	{
		static Type instance;
		return instance;
	}
};