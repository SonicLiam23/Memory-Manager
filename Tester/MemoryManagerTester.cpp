#include "MemoryManagerTester.h"
#include <chrono>
#include "MemoryManager.h"
#include <sstream>
#include <string>
#include <queue>
#include <iostream>
using namespace std::chrono;

#define LOGTEXTM(x)

void MemoryManagerTester::TestAll()
{
	ZeroAllocationTest();
	BulkAllocate();
	BulkAllocateNew();
	OverflowSlab();
	OverflowChunk();
	EnduranceAndDeallocateTest(5);
	LimitTest(10);
}

void MemoryManagerTester::ZeroAllocationTest()
{
	LOGTEXTM("Starting zero allocation test");
	void* ptr = MemoryManager::Get()->AllocateRaw(0);
	
	// if it sucessufy deallocates, then it didnt create "no" memory, test passed!!!
	MemoryManager::Get()->DeallocateRaw(ptr);
	LOGTEXTM("Zero allocation test complete");
	LOGTEXTM("===================");
}

void MemoryManagerTester::BulkAllocate()
{
	constexpr int loopAmount = 10000000;

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
			if (rand() % 2 == 1) MemoryManager::Get()->DeallocateRaw(ptr);
			
		}
		steady_clock::time_point end = steady_clock::now();
		
		float diff = duration_cast<milliseconds>(end - start).count();

		std::stringstream ss;
		ss << "Time taken: " << diff << " milliseconds.";
		LOGTEXTM(ss.str());

		diff = duration_cast<microseconds>(end - start).count();
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

		float diffC = duration_cast<milliseconds>(end - start).count();

		ss.str("");
		ss << "Time taken: " << diffC << " milliseconds.";
		LOGTEXTM(ss.str());

		diffC = duration_cast<microseconds>(end - start).count();
		ss.str("");
		float avgC = diffC / loopAmount;
		ss << "Average time for 1 cycle (allocation + 50% chance of dealloc): " << avgC << " microseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "My memory manager had a time difference of " << avgC - avg << " microseconds on average per cycle.";
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
	size64* ptrs[loopAmount] = { nullptr };

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
			ptr->c[0] = 'x';       // touch memory to avoid lazy page commitment
			ptr->c[63] = 'y';   
			char tmp = ptr->c[0];
			if (rand() % 2 == 1) delete ptr;
			else ptrs[i] = ptr;

			
		}
		std::cout << MemoryManager::fragmentedBytes;
		steady_clock::time_point end = steady_clock::now();
		for (size64* ptr : ptrs)
		{
			if (ptr) 
				delete ptr;
			ptr = nullptr;
		}
		float diff = duration_cast<milliseconds>(end - start).count();

		std::stringstream ss;
		ss << "Time taken: " << diff << " milliseconds.";
		LOGTEXTM(ss.str());

		diff = duration_cast<microseconds>(end - start).count();
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
			ptr->c[0] = 'x';       // touch memory to avoid lazy page commitment
			ptr->c[63] = 'y';
			char tmp = ptr->c[0];
			if (rand() % 2 == 1) ::delete ptr;
			else ptrs[i] = ptr;
		}
		end = steady_clock::now();
		for (size64* ptr : ptrs)
		{
			if (ptr) ::delete ptr;
			ptr = nullptr;
		}

		float diffC = duration_cast<milliseconds>(end - start).count();

		ss.str("");
		ss << "Time taken: " << diffC << " milliseconds.";
		LOGTEXTM(ss.str());

		diffC = duration_cast<microseconds>(end - start).count();
		ss.str("");
		float avgC = diffC / loopAmount;
		ss << "Average time for 1 cycle (allocation + 50% chance of dealloc): " << avgC << " microseconds.";
		LOGTEXTM(ss.str());

		ss.str("");
		ss << "My memory manager had a time difference of " << avgC - avg << " microseconds on average per cycle.";
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
		diff += duration_cast<microseconds>(after - before).count();
		
		
		ptrs[i] = ptr;
	}

	std::stringstream ss;
	ss << "Slab overflow allocation completed in: " << diff << " microseconds.";
	LOGTEXTM(ss.str());
	LOGTEXTM("Starting deallocation...");
	for (int i = 0; i < loopAmount; ++i)
	{
		before = steady_clock::now();
		MemoryManager::Get()->DeallocateRaw(ptrs[i]);
		after = steady_clock::now();
		diff += duration_cast<microseconds>(after - before).count();
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
	constexpr int loopAmount = 1000;
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

	float diff = duration_cast<milliseconds>(end - start).count();
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
	diff = duration_cast<microseconds>(end - start).count();
	ss << "Chunk overflow deallocation completed in: " << diff << " microseconds.";
	LOGTEXTM(ss.str());
	LOGTEXTM("Chunk overflow test complete.");
	LOGTEXTM("===================\n");
}

void MemoryManagerTester::LimitTest(int secs)
{

	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));
	LOGTEXTM("Starting limit test...");
	long int allocs = 0;

	auto start = steady_clock::now();
	auto end = start + seconds(secs);
	float diffAlloc = 0;

	steady_clock::time_point before;
	steady_clock::time_point after;

	while (steady_clock::now() < end) 
	{
		size_t size = rand() % 1028 + 1;
		before = steady_clock::now();
		void* ptr = MemoryManager::Get()->AllocateRaw(size);
		after = steady_clock::now();
		diffAlloc += duration_cast<microseconds>(after - before).count();
		++allocs;
	}
	std::stringstream ss;
	LOGTEXTM("Limit test complete!");

	ss << "Allocated " << allocs << " pieces of data";
	LOGTEXTM(ss.str());
	ss.str("");

	float avg = diffAlloc / allocs;
	ss << "Average of " << avg << " microseconds per allocation.";
	LOGTEXTM(ss.str());
	ss.str("");
	LOGTEXTM("===================\n");
}

void MemoryManagerTester::EnduranceAndDeallocateTest(int mins)
{
	//creates it if it doesnt exist
	MemoryManager::Get();
	srand(time(NULL));
	LOGTEXTM("Starting endurance test...");
	long int allocs = 0;

	auto start = steady_clock::now();
	std::queue<steady_clock::time_point> deallocTimes;
	std::queue<void*> deallocData;
	auto end = start + minutes(mins);
	float diffAlloc = 0;
	float diffDeAlloc = 0;

	steady_clock::time_point before;
	steady_clock::time_point after;

	while (steady_clock::now() < end)
	{
		size_t size = rand() % 1028 + 1;
		before = steady_clock::now();
		void* ptr = MemoryManager::Get()->AllocateRaw(size);
		// size64* ptr = new size64();
		after = steady_clock::now();
		diffAlloc += duration_cast<microseconds>(after - before).count();
		++allocs;
		

		// "mimic" deallocation some time after allocation
		deallocTimes.push(steady_clock::now() + milliseconds(rand() % 2000));
		deallocData.push(ptr);
		auto now = steady_clock::now();

		// Process ALL deallocations whose scheduled time has passed
		while (!deallocTimes.empty() && now >= deallocTimes.front())
		{
			void* dealloc = deallocData.front();

			before = steady_clock::now();
			MemoryManager::Get()->DeallocateRaw(dealloc);
			after = steady_clock::now();

			diffDeAlloc += duration_cast<microseconds>(after - before).count();

			deallocData.pop();
			deallocTimes.pop();
		}
	}

	// clean up rest of data
	while (!deallocData.empty())
	{
		if (steady_clock::now() >= deallocTimes.front())
		{
			void* dealloc = deallocData.front();
			before = steady_clock::now();
			MemoryManager::Get()->DeallocateRaw(dealloc);
			after = steady_clock::now();
			diffDeAlloc += duration_cast<microseconds>(after - before).count();
			deallocData.pop();
			deallocTimes.pop();
		}
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


	avg = diffDeAlloc / allocs;
	ss << "Average of " << avg << " microseconds per deallocation.";
	LOGTEXTM(ss.str()); 
	ss.str("");
	LOGTEXTM("===================\n");
}
