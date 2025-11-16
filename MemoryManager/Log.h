#pragma once
#include <iostream>
struct Log
{
	template <typename T>
	static void LogText(T text);
};

template<typename T>
void Log::LogText(T text)
{
	try
	{
		std::cout << text << std::endl;
	}
	catch (...)
	{

	}
}

