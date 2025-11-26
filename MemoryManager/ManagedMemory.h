#pragma once
#include "MemoryManager.h"
#include "Slab.h"
#include "Log.h"
// new --> placementNew(operatorNew(sizeof(type)), Type(args))
struct ManagedMemory
{
    void* operator new(size_t size)
    {
        if (size == 0) ++size;
        return MemoryManager::Get()->AllocateRaw(size);
    
    }

    void operator delete(void* ptr)
    {
        if (!ptr) return;
        MemoryManager::Get()->DeallocateRaw(ptr);
    }

    void* operator new[](size_t size)
    {
        if (size == 0) ++size;
        return MemoryManager::Get()->AllocateRaw(size);
    }

    void operator delete[](void* ptr)
    {
        if (!ptr) return;
        Slab::ChunkNode* node = (Slab::ChunkNode*)ptr; // ptr is start of data
        MemoryManager::Get()->DeallocateRaw(ptr);
    }
};

