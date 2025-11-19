#pragma once
#if defined _DEBUG
#include <string>
#include <sstream>
#include <fstream>

#define LOGTEXT(x) Log::LogText(__FILE__, __LINE__, x)
#define LOGTEXTM(x) Log::LogTextMinimal(x)


#else

#define LOGTEXT(x)
#define LOGTEXTM(x)

#endif

#include <iostream>

struct Log
{
	template <typename T>
	static void LogText(const char* file, int line, T text);

	template <typename T>
	static void LogTextMinimal(T text);

	

	static void EndLog()
	{
		std::ofstream file("log.txt");
		file << logText.str();
		file.close();
	}

private:
	static std::stringstream logText;
};

template<typename T>
static void Log::LogText(const char* file, int line, T text)
{
	std::stringstream t;
	try
	{
		
		t << file << " On line " << line << ": " << text << std::endl;
		LogTextMinimal(t.str());
	}
	catch(...) {}
}

template <typename T>
static void Log::LogTextMinimal(T text)
{
	try
	{
		std::cout << text << std::endl;
		logText << text << "\n";
	}
	catch(...) {}
}