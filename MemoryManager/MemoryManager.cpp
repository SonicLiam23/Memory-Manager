#include "MemoryManager.h"
#include "ManagedMemory.h"
#include <cstdlib>
#include <new>
#include <cassert>
size_t MemoryManager::fragmentedBytes = 0;

MemoryManager* MemoryManager::s_instance = nullptr;

MemoryManager* MemoryManager::Get()
{
    return s_instance ? s_instance : (s_instance = new MemoryManager());
}

MemoryManager::MemoryManager()
{
    nextPtr = malloc(BLOCK_SIZE);
    assert(nextPtr != nullptr);
    endOfMemory = reinterpret_cast<uintptr_t>(nextPtr) + BLOCK_SIZE;
    blocks.push_back(nextPtr);

    for (size_t i = 0; i < NUM_SIZE_CLASSES; ++i)
        currentSlabs[i] = CreateNewSlab(sizeClasses[i]);
}

void MemoryManager::CreateNewBlock(size_t minSizeBytes)
{
    void* raw = malloc(minSizeBytes > BLOCK_SIZE ? minSizeBytes : BLOCK_SIZE);
    assert(raw != nullptr);
    nextPtr = raw;
    endOfMemory = reinterpret_cast<uintptr_t>(raw) + (minSizeBytes > BLOCK_SIZE ? minSizeBytes : BLOCK_SIZE);
    blocks.push_back(raw);
}

Slab* MemoryManager::CreateNewSlab(size_t chunkSize, size_t minSlabBytes)
{
    size_t alignment = alignof(std::max_align_t);
    size_t fullChunkSize = sizeof(Slab::ChunkNode) + chunkSize;
    fullChunkSize = (fullChunkSize + alignment - 1) & ~(alignment - 1);

    // get how many chunks fit in slab
    size_t chunkCount = (minSlabBytes - sizeof(Slab)) / fullChunkSize;
    if (chunkCount == 0) chunkCount = 1;

    // get  total slab memory needed
    size_t slabMemoryNeeded = sizeof(Slab) + fullChunkSize * chunkCount + alignment;

    if ((uintptr_t)nextPtr + slabMemoryNeeded > endOfMemory)
        CreateNewBlock(slabMemoryNeeded);

    Slab* slab = new(nextPtr) Slab(chunkSize, (unsigned int)chunkCount);
    nextPtr = (void*)((uintptr_t)nextPtr + slabMemoryNeeded);
    return slab;
}

size_t MemoryManager::SizeToClass(size_t size)
{
    for (size_t i = 0; i < NUM_SIZE_CLASSES; ++i)
        if (size <= sizeClasses[i]) return i;
    return NUM_SIZE_CLASSES - 1;
}

void* MemoryManager::AllocateRaw(size_t size)
{
    if (size < 1) size == 1;

    // Large allocation bypass
    if (size > sizeClasses[NUM_SIZE_CLASSES - 1])
    {
        size_t totalSize = sizeof(Slab::ChunkNode) + size;
        Slab::ChunkNode* node = (Slab::ChunkNode*)malloc(totalSize);
        node->parentSlab = nullptr;
        return node + 1;
    }

    size_t classIdx = SizeToClass(size);
    Slab* slab = currentSlabs[classIdx];

    

    // Pick a usable slab if current is null or full
    if (!slab || slab->isFull || !slab->freeListHead)
    {
        slab = PickPartialSlab(classIdx);

        if (!slab)
        {
            // no partial slab available, create new
            slab = CreateNewSlab(sizeClasses[classIdx]);
        }

        currentSlabs[classIdx] = slab;
    }

    assert(slab != nullptr);
    MemoryManager::fragmentedBytes += slab->ChunkSize - size  ;

    void* chunk = slab->Allocate();
    assert(PointerInBlocks(chunk) && "Chunk allocated outside any preallocated block!");

    // If slab is now full, move to full list
    if (slab->isFull)
    {
        AddToFullList(classIdx, slab);
        currentSlabs[classIdx] = nullptr;
    }

    // add data to the node
    Slab::ChunkNode* node = (Slab::ChunkNode*)chunk;
    node = node - 1; // get the actual node, chunk is the start of the data
    node->dataSize = size;
    
    return chunk;
}

void MemoryManager::DeallocateRaw(void* chunk)
{
    if (!chunk) return;

    Slab::ChunkNode* node = ((Slab::ChunkNode*)chunk) - 1;

    // Dedicated allocation
    if (!node->parentSlab)
    {
        free(node);
        return;
    }

    Slab* slab = node->parentSlab;

    bool wasFull = slab->isFull;
    slab->DeallocateRaw(chunk);
    MemoryManager::fragmentedBytes -= slab->ChunkSize - node->dataSize;

    size_t classIdx = SizeToClass(slab->ChunkSize);

    // If slab transitioned FULL → PARTIAL
    if (wasFull && !slab->isFull)
    {
        RemoveFromFullList(classIdx, slab);
        AddToPartialList(classIdx, slab);
    }

    // If slab was already partial but not tracked (rare)
    else if (!slab->isFull && !slab->inPartialList)
    {
        AddToPartialList(classIdx, slab);
    }
}

void MemoryManager::AddToPartialList(size_t idx, Slab* slab)
{
    // Remove first if already in the list
    if (slab->inPartialList)
        RemoveFromPartialList(idx, slab);

    slab->nextPartial = partialSlabs[idx];
    slab->prevPartial = nullptr;
    slab->inPartialList = true;

    if (partialSlabs[idx])
        partialSlabs[idx]->prevPartial = slab;

    partialSlabs[idx] = slab;
}

void MemoryManager::RemoveFromPartialList(size_t idx, Slab* slab)
{
    if (!slab->inPartialList) return;

    if (slab->prevPartial)
        slab->prevPartial->nextPartial = slab->nextPartial;
    else
        partialSlabs[idx] = slab->nextPartial;

    if (slab->nextPartial)
        slab->nextPartial->prevPartial = slab->prevPartial;

    slab->nextPartial = nullptr;
    slab->prevPartial = nullptr;
    slab->inPartialList = false;
}


void MemoryManager::AddToFullList(size_t idx, Slab* slab)
{
    slab->nextFull = fullSlabs[idx];
    slab->prevFull = nullptr;

    if (fullSlabs[idx])
        fullSlabs[idx]->prevFull = slab;

    fullSlabs[idx] = slab;
}

void MemoryManager::RemoveFromFullList(size_t idx, Slab* slab)
{
    if (slab->prevFull)
        slab->prevFull->nextFull = slab->nextFull;
    else
        fullSlabs[idx] = slab->nextFull;

    if (slab->nextFull)
        slab->nextFull->prevFull = slab->prevFull;

    slab->nextFull = slab->prevFull = nullptr;
}

Slab* MemoryManager::PickPartialSlab(size_t classIdx)
{
    Slab* iter = partialSlabs[classIdx];
    while (iter)
    {
        if (iter->freeListHead && !iter->isFull)
        {
            // remove from partial list before returning
            RemoveFromPartialList(classIdx, iter);
            return iter;
        }
        iter = iter->nextPartial;
    }
    return nullptr; // no usable partial slab
}

MemoryManager::~MemoryManager()
{
    for (void* ptr : blocks) free(ptr);
    for (Slab* slab : currentSlabs) slab->~Slab();
    for (Slab* slab : partialSlabs) slab->~Slab();
    for (Slab* slab : fullSlabs) slab->~Slab();
}

void MemoryManager::Reset()
{
    MemoryManager::fragmentedBytes = 0;
    if (!s_instance) return;
    delete s_instance;
    s_instance = nullptr;
}

#ifdef _DEBUG
bool MemoryManager::PointerInBlocks(void* ptr) {
    uintptr_t uptr = reinterpret_cast<uintptr_t>(ptr);
    for (void* block : blocks) {
        uintptr_t bstart = reinterpret_cast<uintptr_t>(block);
        uintptr_t bend = bstart + BLOCK_SIZE;
        if (uptr >= bstart && uptr < bend) return true;
    }
    return false;
}
#endif // _DEBUG


