#pragma once
#if defined _DEBUG
#include <string>
#include <sstream>

#define LOGTEXT(x) Log::LogText(__FILE__, __LINE__, x)
#define LOGTEXTM(x) Log::LogTextMinimal(x)


#else

#define LOGTEXT(x)

#endif

#include <iostream>

struct Log
{
	template <typename T>
	static void LogText(const char* file, int line, T text);

	template <typename T>
	static void LogTextMinimal(T text);
};

template<typename T>
static void Log::LogText(const char* file, int line, T text)
{
	try
	{
		std::cout << file << " On line " << line << ": " << text << std::endl;
	}
	catch (...)
	{

	}
}

template <typename T>
static void Log::LogTextMinimal(T text)
{
	try
	{
		std::cout << text << std::endl;
	}
	catch (...)
	{

	}
}