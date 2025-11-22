#include "MemoryManager.h"

#include "ManagedMemory.h"
#include "Log.h"
#include "windows.h"
#include <chrono>
#include <memory>
using namespace std::chrono;

struct myInt : public ManagedMemory
{
	
	myInt() = default;
	char i[16];
};

int main()
{
	int* num1 = MemoryManager::Get()->AllocateAndCreate<int>();
	int* num2 = MemoryManager::Get()->AllocateAndCreate<int>();
	int* num3 = MemoryManager::Get()->AllocateAndCreate<int>();
	LOGTEXTM((void*)num1);
	LOGTEXTM((void*)num2);
	LOGTEXTM((void*)num3);
	LOGTEXTM("========");
	MemoryManager::Get()->DestroyAndDeallocate<int>(num2);
	int* num4 = MemoryManager::Get()->AllocateAndCreate<int>();
	LOGTEXTM((void*)num4);
	LOGTEXTM("========");
	char* char1 = MemoryManager::Get()->AllocateAndCreate<char>();
	char* char2 = MemoryManager::Get()->AllocateAndCreate<char>();
	char* char3 = MemoryManager::Get()->AllocateAndCreate<char>();
	char* char4 = MemoryManager::Get()->AllocateAndCreate<char>();
	LOGTEXTM((void*)char1);
	LOGTEXTM((void*)char2);
	LOGTEXTM((void*)char3);
	LOGTEXTM((void*)char4);
	LOGTEXTM("========");
	MemoryManager::Get()->DestroyAndDeallocate<char>(char1);
	MemoryManager::Get()->DestroyAndDeallocate<char>(char2);
	char* char5 = MemoryManager::Get()->AllocateAndCreate<char>();
	char* char6 = MemoryManager::Get()->AllocateAndCreate<char>();
	LOGTEXTM((void*)char5);
	LOGTEXTM((void*)char6);
	LOGTEXTM("========");

	while (true)
	{
		//MemoryManager::Get()->AllocateAndCreate<int>();
		new myInt();
		LOGTEXTM("Data placed");
		
	}

	delete MemoryManager::Get();
	Log::EndLog();

	return 0;
}

/*
void m(){


	srand(time(NULL));
	void* address = malloc(4);
	int* out = new(address) int(8);

	int wins = 0;
	int losses = 0;
	int draws = 0;

	for (int j = 0; j < 2000; ++j)
	{

		std::cout << "Commencing Cast Test\n";


		system_clock::time_point t2;
		system_clock::time_point t = system_clock::now();


		for (int i = 0; i < 1000000; ++i)
		{


			bool b = *(reinterpret_cast<bool*>(address));
		}

		t2 = system_clock::now();

		float diff = system_clock::duration(t2 - t).count();

		std::cout << "Entering Stage 2\n";

		t = system_clock::now();


		for (int i = 0; i < 1000000; ++i)
		{


			bool b = *((bool*)address);
		}


		t2 = system_clock::now();


		float diff2 = system_clock::duration(t2 - t).count();

		std::cout << "C-Style Cast Time: " << diff << "\nReinterpret Time: " << diff2 << std::endl << std::endl << std::endl;

		if (diff < diff2) ++wins;
		else if (diff > diff2) ++losses;
		else ++draws;

	}

	std::cout << "C-Style: " << wins << "\nReinterpret: " << losses << "\nDraws: " << draws << "\n";

	system("pause");
}
*/
