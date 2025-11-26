#include "Slab.h"

Slab::Slab(size_t chunkSize, unsigned int chunkAmount) : ChunkSize(chunkSize), isFull(false), freeListHead(nullptr), inPartialList(false)
{
    size_t alignment = alignof(std::max_align_t);
    fullChunkSize = sizeof(ChunkNode) + chunkSize;
    fullChunkSize = (fullChunkSize + alignment - 1) & ~(alignment - 1);

    uintptr_t rawStart = reinterpret_cast<uintptr_t>(this) + sizeof(Slab);
    uintptr_t alignedStart = (rawStart + alignment - 1) & ~(alignment - 1);
    start = reinterpret_cast<void*>(alignedStart);
    end = (Byte*)start + fullChunkSize * chunkAmount;

#ifdef CORRECT_REUSE_CHECK
    size_t size = reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(start);
    std::memset(start, 0x55, size);
#endif

    Byte* ptr = (Byte*)start;
    for (unsigned int i = 0; i < chunkAmount; ++i)
    {
        ChunkNode* node = reinterpret_cast<ChunkNode*>(ptr);
        node->parentSlab = this;
        node->next = freeListHead;
        freeListHead = node;

        ptr += fullChunkSize;
    }

}

void* Slab::Allocate()
{
    if (!freeListHead) {
        isFull = true;
        return nullptr; 
    }

    ChunkNode* node = freeListHead;
    freeListHead = node->next;

    isFull = (freeListHead == nullptr);

    void* userptr = reinterpret_cast<void*>(node + 1);

#ifdef _DEBUG
    // make sure userPtr is within slab memory
    uintptr_t uptr = reinterpret_cast<uintptr_t>(userptr);
    uintptr_t slabStart = reinterpret_cast<uintptr_t>(start);
    uintptr_t slabEnd = reinterpret_cast<uintptr_t>(end);
    assert(uptr >= slabStart && "Allocation before slab start!");
    assert(uptr + ChunkSize <= slabEnd && "Allocation past slab end!");
    assert(reinterpret_cast<uintptr_t>(userptr) >= reinterpret_cast<uintptr_t>(start));
    assert(reinterpret_cast<uintptr_t>(userptr) + ChunkSize <= reinterpret_cast<uintptr_t>(end));
#endif

#ifdef CORRECT_REUSE_CHECK
    // Check if memory is still marked as freed
    assert(*(uint32_t*)userptr == 0x55555555 && "Chunk overwritten");

    // Now mark it as allocated
    std::memset(userptr, 0xAA, ChunkSize);
#endif
    // Return pointer right after the header
    return (void*)(node + 1);
}

void Slab::DeallocateRaw(void* userPtr)
{
#ifdef _DEBUG
    assert(userPtr != nullptr);
    uintptr_t uptr = reinterpret_cast<uintptr_t>(userPtr);
    assert(uptr >= reinterpret_cast<uintptr_t>(start) && uptr + ChunkSize <= reinterpret_cast<uintptr_t>(end) && "Deallocating memory outside of slab!");
#endif
    if (!userPtr) return;

#ifdef CORRECT_REUSE_CHECK
    std::memset(userPtr, 0x55, ChunkSize); // mark as freed
#endif



    ChunkNode* node = ((ChunkNode*)userPtr) - 1;

    node->next = freeListHead;
    freeListHead = node;

    isFull = false;
}