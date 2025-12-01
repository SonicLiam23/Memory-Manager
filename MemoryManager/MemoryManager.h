#pragma once
#include "Slab.h"
#include "ManagerSettings.h"
#include <vector>
#include <cstddef>
#include <cassert>
#include <new>

class MemoryManager
{
private:
    static constexpr size_t NUM_SIZE_CLASSES = 13; // 1B,2B,4B,...,4096B
    size_t sizeClasses[NUM_SIZE_CLASSES] = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096 };

    Slab* currentSlabs[NUM_SIZE_CLASSES] = { nullptr };
    Slab* partialSlabs[NUM_SIZE_CLASSES] = { nullptr };
    Slab* fullSlabs[NUM_SIZE_CLASSES] = { nullptr };

    std::vector<void*> blocks;
    void* nextPtr = nullptr;
    uintptr_t endOfMemory;

    MemoryManager();
    void CreateNewBlock(size_t minSizeBytes);
    Slab* CreateNewSlab(size_t chunkSize, size_t slabSizeBytes = BLOCK_SIZE);
    size_t SizeToClass(size_t size);

    void AddToPartialList(size_t idx, Slab* slab);
    void RemoveFromPartialList(size_t idx, Slab* slab);
    void AddToFullList(size_t idx, Slab* slab);
    void RemoveFromFullList(size_t idx, Slab* slab);
    Slab* PickPartialSlab(size_t classIdx);

#ifdef _DEBUG
    bool PointerInBlocks(void* ptr);
#endif

    static MemoryManager* s_instance;

public:
    static size_t fragmentedBytes;
    static MemoryManager* Get();
    void* AllocateRaw(size_t size);
    void DeallocateRaw(void* chunk);

    template <typename T, typename... Args>
    T* AllocateAndCreate(Args&&... args)
    {
        void* chunk = AllocateRaw(sizeof(T));
        return new(chunk) T(std::forward<Args>(args)...);
    }

    template <typename T>
    void DestroyAndDeallocate(T* obj)
    {
        if (!obj) return;
        obj->~T();
        DeallocateRaw(obj);
    }

    ~MemoryManager();
    static void Reset();
};

