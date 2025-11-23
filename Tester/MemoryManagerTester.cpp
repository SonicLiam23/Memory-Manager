#include "MemoryManagerTester.h"
#include <chrono>
#include "MemoryManager.h"
#include "Log.h"
#include <sstream>
#include <string>
using namespace std::chrono;

void MemoryManagerTester::BulkAllocate()
{
	constexpr int loopAmount = 100000;

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));

	LOGTEXTM("Starting raw bulk allocate test...");


	try
	{
		LOGTEXTM("Starting My Memory manager test...");
		steady_clock::time_point start = steady_clock::now();
		for (int i = 0; i < loopAmount; ++i)
		{
			size_t size = rand() % 1028 + 1;
			void* ptr = MemoryManager::Get()->AllocateRaw(size);
			if (rand() % 2 == 1) MemoryManager::Get()->DeallocateRaw(ptr, size);
		}
		steady_clock::time_point end = steady_clock::now();

		float diff = duration_cast<std::chrono::milliseconds>(end - start).count();

		std::stringstream ss;
		ss << "Time taken: " << diff << " milliseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "Average time for 1 allocation: " << diff / loopAmount << " milliseconds.";
		LOGTEXTM(ss.str());
		//////////////////////////////////////////
		LOGTEXTM("Starting C++ operator new test...");
		start = steady_clock::now();
		for (int i = 0; i < loopAmount; ++i)
		{
			size_t size = rand() % 1028 + 1;
			void* ptr = ::operator new(size);
			// i know some memory will leak here, but it is outside of my manager, so it should be okay as its testing purposes only.
			if (rand() % 2 == 1) ::operator delete(ptr, size);
		}
		end = steady_clock::now();

		float diffC = duration_cast<std::chrono::milliseconds>(end - start).count();

		ss.str("");
		ss << "Time taken: " << diffC << " milliseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "Average time for 1 allocation: " << diffC / loopAmount << " milliseconds.";
		LOGTEXTM(ss.str());


		ss.str("");
		ss << "My memory manager had a time difference of " << diffC - diff << " milliseconds.";
		LOGTEXTM(ss.str());
	}
	catch (...)
	{
		LOGTEXTM("Bulk Allocate test failed.");

	}
	LOGTEXTM("===================");
}

void MemoryManagerTester::BulkAllocateNew()
{
	constexpr int loopAmount = 100000;

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));

	LOGTEXTM("Starting typed bulk allocate test...");


	try
	{
		LOGTEXTM("Starting My Memory manager test...");
		steady_clock::time_point start = steady_clock::now();
		for (int i = 0; i < loopAmount; ++i)
		{
			size_t size = rand() % 1028 + 1;
			size64* ptr = new size64();
			if (rand() % 2 == 1) delete ptr;
		}
		steady_clock::time_point end = steady_clock::now();

		float diff = duration_cast<std::chrono::milliseconds>(end - start).count();

		std::stringstream ss;
		ss << "Time taken: " << diff << " milliseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "Average time for 1 allocation: " << diff / loopAmount << " milliseconds.";
		LOGTEXTM(ss.str());
		//////////////////////////////////////////
		LOGTEXTM("Starting C++ operator new test...");
		start = steady_clock::now();
		for (int i = 0; i < loopAmount; ++i)
		{
			size_t size = rand() % 1028 + 1;
			size64* ptr = ::new size64;
			// i know some memory will leak here, but it is outside of my manager, so it should be okay as its testing purposes only.
			if (rand() % 2 == 1) ::delete ptr;
		}
		end = steady_clock::now();

		float diffC = duration_cast<std::chrono::milliseconds>(end - start).count();

		ss.str("");
		ss << "Time taken: " << diffC << " milliseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "Average time for 1 allocation: " << diffC / loopAmount << " milliseconds.";
		LOGTEXTM(ss.str());


		ss.str("");
		ss << "My memory manager had a time difference of " << diffC - diff << " milliseconds.";
		LOGTEXTM(ss.str());
	}
	catch (...)
	{
		LOGTEXTM("Bulk typed Allocate test failed.");

	}
	LOGTEXTM("===================\n");
}

void MemoryManagerTester::OverflowSlab()
{
	// try overflowing slabs a LOT, by steadily increasing size
	constexpr int loopAmount = 10000;
	// for deallocation
	void* ptrs[loopAmount];

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));

	LOGTEXTM("Starting Slab overflow allocation test...");

	steady_clock::time_point start = steady_clock::now();
	for (int i = 0; i < loopAmount; ++i)
	{
		size_t size = i;
		void* ptr = MemoryManager::Get()->AllocateRaw(i);
		
		ptrs[i] = ptr;
	}
	steady_clock::time_point end = steady_clock::now();
}
