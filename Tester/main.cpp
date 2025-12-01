#include "MemoryManagerTester.h"
#include "MemoryManager.h"
#include <iostream>

int main()
{


	MemoryManagerTester test;

	test.ZeroAllocationTest();

    // reset everything
	MemoryManager::Reset();
	test.BulkAllocate();

	
	MemoryManager::Reset();
	test.BulkAllocateNew();

	MemoryManager::Reset();
	test.OverflowSlab();

	MemoryManager::Reset();
	test.OverflowChunk();

	MemoryManager::Reset();
	test.EnduranceAndDeallocateTest(5);

	MemoryManager::Reset();
	// its a short time, but memory gets used FAST
	// i tested it with 30 seconds and it worked, but I reduced it for the health of your (our) drives xD
	test.LimitTest(10);

	test.TestAll();

	


	delete MemoryManager::Get();

	std::cin.get();
	return 0;
}