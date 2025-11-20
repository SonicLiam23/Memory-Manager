#pragma once
#include "MemoryManager.h"
#include "Log.h"
// new --> placementNew(operatorNew(sizeof(type)), Type(args))
struct ManagedMemory
{
	void* operator new(size_t size)
	{
		if (size == 0) ++size;

		return MemoryManager::Get()->AllocateRaw(size);
	}

	void operator delete(void* ptr, size_t size)
	{
		MemoryManager::Get()->DeallocateRaw(ptr, size);
	}

	void* operator new[](size_t size)
	{
		if (size == 0) ++size;  // Handle zero size allocation for arrays
		return MemoryManager::Get()->AllocateRaw(size);
	}

	void operator delete[](void* ptr, size_t size)
	{
		MemoryManager::Get()->DeallocateRaw(ptr, size);
	}
};

