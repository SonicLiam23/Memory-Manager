#pragma once
#include "ManagedMemory.h"
struct MemoryManagerTester
{
	void TestAll();
	////////////////////

	void ZeroAllocationTest();
	void BulkAllocate();
	void BulkAllocateNew();
	void OverflowSlab();
	// allocates 10000000 bytes of memory, a lot
	void OverflowChunk();

	void LimitTest(int secs);
	void EnduranceAndDeallocateTest(int mins);

};

struct size4 : public ManagedMemory
{
	char c[4];
};


struct size8 : public ManagedMemory
{
	char c[8];
};

struct size64 : public ManagedMemory
{
	char c[65];
};

struct size128 : public ManagedMemory
{
	char c[128];
};

struct size512 : public ManagedMemory
{
	char c[512];
};

struct size1024 : public ManagedMemory
{
	char c[1024];
};

struct sizeBIG : public ManagedMemory
{
	char c[10000000]; // ~10MB
};
