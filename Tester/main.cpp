#include "MemoryManagerTester.h"
#include "MemoryManager.h"
#include <iostream>

int main()
{
	LOGTEXTM("I have enabled logging in release just to show results. This would be disabled in reality.");

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

	LOGTEXTM("\n\nNow to test them without resetting anything, all back-to-back!\n\n");
	test.TestAll();

	

	LOGTEXTM("Deallocating (takes a while in release), you CAN just close this window and view the log if you don't want to wait and skip de-allocation");
	delete MemoryManager::Get();
	LOGTEXTM("Done! Press enter to exit!");
	Log::EndLog();

	std::cin.get();
	return 0;
}