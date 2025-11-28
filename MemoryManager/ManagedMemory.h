#pragma once
#include "MemoryManager.h"
#include "Slab.h"
#include "Log.h"
// new --> placementNew(operatorNew(sizeof(type)), Type(args))
struct ManagedMemory
{
    static int allocations;
    void* operator new(size_t size)
    {
        if (size == 0) ++size;
        allocations++;
        return MemoryManager::Get()->AllocateRaw(size);
        
    }

    void operator delete(void* ptr)
    {
        if (!ptr) return;
        allocations--;
        MemoryManager::Get()->DeallocateRaw(ptr);
    }

    void* operator new[](size_t size)
    {
        if (size == 0) ++size;
        allocations++;
        return MemoryManager::Get()->AllocateRaw(size);
    }

    void operator delete[](void* ptr)
    {
        if (!ptr) return;
        allocations--;
        Slab::ChunkNode* node = (Slab::ChunkNode*)ptr; // ptr is start of data
        MemoryManager::Get()->DeallocateRaw(ptr);
    }
};

