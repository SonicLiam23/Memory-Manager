#pragma once
#include "ManagedMemory.h"
struct MemoryManagerTester
{
	void TestAll();
	////////////////////

	void BulkAllocate();
	void BulkAllocateNew();
	void OverflowSlab();
	void OverflowChunk();
	void MassiveDataAllocation();

};

class size4 : public ManagedMemory
{
	char c[4];
};


class size8 : public ManagedMemory
{
	char c[8];
};

class size64 : public ManagedMemory
{
	char c[64];
};

class size128 : public ManagedMemory
{
	char c[128];
};

class size512 : public ManagedMemory
{
	char c[512];
};

class size1024 : public ManagedMemory
{
	char c[1024];
};

class sizeBIG : public ManagedMemory
{
	char c[10000000]; // ~10MB
};
