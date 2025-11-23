#include "MemoryManagerTester.h"
#include "MemoryManager.h"

int main()
{
	MemoryManagerTester test;
	test.BulkAllocate();

	// reset everything
	MemoryManager::Get()->Reset();
	test.BulkAllocateNew();

	MemoryManager::Get()->Reset();
	test.OverflowSlab();

	MemoryManager::Get()->Reset();
	test.OverflowChunk();

	MemoryManager::Get()->Reset();
	test.EnduranceTest(30);

	LOGTEXTM("\n\nNow to test them without resetting anything, all back-to-back!\n\n");
	test.TestAll();

	Log::EndLog();
	delete MemoryManager::Get();
	return 0;
}