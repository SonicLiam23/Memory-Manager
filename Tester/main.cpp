#include "MemoryManagerTester.h"
#include "MemoryManager.h"

int main()
{
	MemoryManagerTester test;
	test.BulkAllocate();
	// reset everything
	MemoryManager::Get()->Reset();
	test.BulkAllocateNew();

	return 0;
}