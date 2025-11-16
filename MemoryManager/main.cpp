#include <memory>
#include "MemoryManager.h"
#include "Log.h"

int main()
{ 
	MemoryManager m;

	int* num1 = m.CreateAndAllocate<int>();
	int* num2 = m.CreateAndAllocate<int>();
	int* num3 = m.CreateAndAllocate<int>();
	Log::LogText((void*)num1);
	Log::LogText((void*)num2);
	Log::LogText((void*)num3);
	Log::LogText("========");
	m.DestroyAndDeallocate<int>(num2);
	int* num4 = m.CreateAndAllocate<int>();
	Log::LogText((void*)num4);
	Log::LogText("========");
	char* char1 = m.CreateAndAllocate<char>();
	char* char2 = m.CreateAndAllocate<char>();
	char* char3 = m.CreateAndAllocate<char>();
	char* char4 = m.CreateAndAllocate<char>();
	Log::LogText((void*)char1);
	Log::LogText((void*)char2);
	Log::LogText((void*)char3);
	Log::LogText((void*)char4);
	Log::LogText("========");
	m.DestroyAndDeallocate<char>(char1);
	m.DestroyAndDeallocate<char>(char2);
	char* char5 = m.CreateAndAllocate<char>();
	char* char6 = m.CreateAndAllocate<char>();
	Log::LogText((void*)char5);
	Log::LogText((void*)char6);
	Log::LogText("========");

	return 0;
}