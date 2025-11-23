#include "MemoryManagerTester.h"
#include <chrono>
#include "MemoryManager.h"
#include "Log.h"
#include <sstream>
#include <string>
using namespace std::chrono;

void MemoryManagerTester::TestAll()
{
	BulkAllocate();
	BulkAllocateNew();
	OverflowSlab();
	OverflowChunk();
	EnduranceTest(30);
}

void MemoryManagerTester::BulkAllocate()
{
	constexpr int loopAmount = 100000;

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));
	int seed = rand();

	LOGTEXTM("Starting raw bulk allocate test...");
	try
	{
		srand(seed);
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

		diff = duration_cast<std::chrono::microseconds>(end - start).count();
		ss.str("");
		float avg = diff / loopAmount;
		ss << "Average time for 1 cycle(allocation + 50 % chance of dealloc): " << avg << " microseconds.";
		LOGTEXTM(ss.str());
		//////////////////////////////////////////
		srand(seed);
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

		diffC = duration_cast<std::chrono::microseconds>(end - start).count();
		ss.str("");
		float avgC = diffC / loopAmount;
		ss << "Average time for 1 cycle (allocation + 50% chance of dealloc): " << avgC << " microseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "My memory manager had a time difference of " << avgC - avg << " microeconds on average per cycle.";
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
	size64* ptrs[loopAmount];

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));
	int seed = rand();

	LOGTEXTM("Starting typed bulk allocate test...");


	try
	{
		srand(seed);
		LOGTEXTM("Starting My Memory manager test...");
		steady_clock::time_point start = steady_clock::now();
		for (int i = 0; i < loopAmount; ++i)
		{
			size64* ptr = new size64();
			if (rand() % 2 == 1) delete ptr;
			else ptrs[i] = ptr;
		}
		steady_clock::time_point end = steady_clock::now();
		for (size64* ptr : ptrs)
		{
			if (ptr) delete ptr;
			ptr = nullptr;
		}

		float diff = duration_cast<std::chrono::milliseconds>(end - start).count();

		std::stringstream ss;
		ss << "Time taken: " << diff << " milliseconds.";
		LOGTEXTM(ss.str());

		diff = duration_cast<std::chrono::microseconds>(end - start).count();
		ss.str("");
		float avg = diff / loopAmount;
		ss << "Average time for 1 cycle(allocation + 50 % chance of dealloc): " << avg << " microseconds.";
		LOGTEXTM(ss.str());
		//////////////////////////////////////////
		srand(seed);
		LOGTEXTM("Starting C++ operator new test...");
		start = steady_clock::now();
		for (int i = 0; i < loopAmount; ++i)
		{
			size64* ptr = ::new size64();
			if (rand() % 2 == 1) ::delete ptr;
			else ptrs[i] = ptr;
		}
		end = steady_clock::now();
		for (size64* ptr : ptrs)
		{
			if (ptr) delete ptr;
			ptr = nullptr;
		}

		float diffC = duration_cast<std::chrono::milliseconds>(end - start).count();

		ss.str("");
		ss << "Time taken: " << diffC << " milliseconds.";
		LOGTEXTM(ss.str());

		diffC = duration_cast<std::chrono::microseconds>(end - start).count();
		ss.str("");
		float avgC = diffC / loopAmount;
		ss << "Average time for 1 cycle (allocation + 50% chance of dealloc): " << avgC << " microseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "My memory manager had a time difference of " << avgC - avg << " microeconds on average per cycle.";
		LOGTEXTM(ss.str());
	}
	catch (...)
	{
		LOGTEXTM("Bulk typed Allocate test failed.");

	}
	LOGTEXTM("===================");
	LOGTEXTM("");
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

	float diff = 0;

	steady_clock::time_point before;
	steady_clock::time_point after;
	LOGTEXTM("Starting Slab overflow test...");

	for (int i = 0; i < loopAmount; ++i)
	{
		size_t size = i;
		before = steady_clock::now();
		void* ptr = MemoryManager::Get()->AllocateRaw(i);
		after = steady_clock::now();
		diff += duration_cast<std::chrono::microseconds>(after - before).count();
		
		
		ptrs[i] = ptr;
	}

	std::stringstream ss;
	ss << "Slab overflow allocation completed in: " << diff << " microseconds.";
	LOGTEXTM(ss.str());
	LOGTEXTM("Starting deallocation...");
	for (int i = 0; i < loopAmount; ++i)
	{
		before = steady_clock::now();
		MemoryManager::Get()->DeallocateRaw(ptrs[i], i);
		after = steady_clock::now();
		diff += duration_cast<std::chrono::microseconds>(after - before).count();
	}
	ss.str("");
	ss << "Slab overflow deallocation completed in: " << diff << " microseconds.";
	LOGTEXTM(ss.str());
	LOGTEXTM("Slab overflow test complete.");
	LOGTEXTM("===================\n");
}

void MemoryManagerTester::OverflowChunk()
{
	// try overflowing chunks a LOT, by steadily increasing size
	constexpr int loopAmount = 100;
	// for deallocation
	sizeBIG* ptrs[loopAmount];

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));

	LOGTEXTM("Starting Chunk overflow test...");

	steady_clock::time_point start = steady_clock::now();
	for (int i = 0; i < loopAmount; ++i)
	{
		ptrs[i] = new sizeBIG();
	}
	steady_clock::time_point end = steady_clock::now();

	float diff = duration_cast<std::chrono::milliseconds>(end - start).count();
	std::stringstream ss;
	ss << "Chunk overflow allocation completed in: " << diff << " milliseconds.";
	LOGTEXTM(ss.str());
	LOGTEXTM("Starting deallocation...");
	start = steady_clock::now();
	for (int i = 0; i < loopAmount; ++i)
	{
		delete ptrs[i];
	}
	end = steady_clock::now();
	ss.str("");
	diff = duration_cast<std::chrono::microseconds>(end - start).count();
	ss << "Chunk overflow deallocation completed in: " << diff << " microseconds.";
	LOGTEXTM(ss.str());
	LOGTEXTM("Chunk overflow test complete.");
	LOGTEXTM("===================\n");
}

void MemoryManagerTester::EnduranceTest(int secs)
{

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));
	LOGTEXTM("Starting endurance test...");
	long int allocs = 0;
	long int deallocs = 0;

	auto start = std::chrono::steady_clock::now();
	auto end = start + std::chrono::seconds(secs);
	float diffAlloc = 0;
	float diffDeAlloc = 0;

	steady_clock::time_point before;
	steady_clock::time_point after;

	while (std::chrono::steady_clock::now() < end) 
	{
		size_t size = rand() % 1028 + 1;
		before = steady_clock::now();
		void* ptr = MemoryManager::Get()->AllocateRaw(size);
		after = steady_clock::now();
		diffAlloc += duration_cast<std::chrono::microseconds>(after - before).count();
		// make it so there is a 90% chance it is deleted, assume user is deleting most of the time
		if (rand() % 10 < 9)
		{
			before = steady_clock::now();
			MemoryManager::Get()->DeallocateRaw(ptr, size);
			after = steady_clock::now();
			diffDeAlloc += duration_cast<std::chrono::microseconds>(after - before).count();
			++deallocs;
		}
		++allocs;
	}
	std::stringstream ss;
	LOGTEXTM("Endurance test complete!");

	ss << "Allocated " << allocs << " pieces of data";
	LOGTEXTM(ss.str());
	ss.str("");

	float avg = diffAlloc / allocs;
	ss << "Average of " << avg << " microseconds per allocation.";
	LOGTEXTM(ss.str());
	ss.str("");

	ss << "DeAllocated " << deallocs << " pieces of data";
	LOGTEXTM(ss.str());
	ss.str("");

	avg = diffDeAlloc / deallocs;
	ss << "Average of " << avg << " microseconds per DeAllocation.";
	LOGTEXTM(ss.str());
	LOGTEXTM("===================\n");
}
