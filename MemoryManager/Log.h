#pragma once
#include <string>
#include <sstream>
#include <fstream>
//#if defined _DEBUG


#define LOGTEXT(x) Log::LogText(__FILE__, __LINE__, x)
#define LOGTEXTM(x) Log::LogTextMinimal(x)


//#else

//#define LOGTEXT(x)
//#define LOGTEXTM(x)

//#endif

#include <iostream>

struct Log
{
	template <typename T>
	static void LogText(const char* file, int line, T text);

	template <typename T>
	static void LogTextMinimal(T text);

	

	static void EndLog()
	{	
		file.close();
	}

private:
	static std::stringstream logText;
	static std::ofstream file;
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
		if (!file.is_open())
		{
			file.open("log.txt");
		}
		std::cout << text << std::endl;
		file << text << "\n";
	}
	catch(...) {}
}