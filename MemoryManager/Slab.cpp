#include "Slab.h"
#include <assert.h>

Slab::Slab(size_t chunkSize, unsigned int chunkAmount) : ChunkSize(chunkSize), isFull(false), freeListHead(nullptr)
{
    // Calculate start of slab memory (after Slab header)
    start = (void*)((Byte*)this + sizeof(Slab));
    end = (void*)((Byte*)start + chunkSize * chunkAmount);

    // Initialize free list: each chunk points to the next
    Byte* ptr = (Byte*)start;
    for (unsigned int i = 0; i < chunkAmount; ++i)
    {
        ChunkNode* node = reinterpret_cast<ChunkNode*>(ptr);
        node->next = freeListHead;
        freeListHead = node;
        ptr += chunkSize;
    }
}

void* Slab::Allocate()
{
    if (!freeListHead)
    {
        isFull = true;
        return nullptr;
    }

    // Pop from free list
    ChunkNode* node = freeListHead;
    freeListHead = freeListHead->next;

    if (!freeListHead)
        isFull = true;

    return node;
}

void Slab::DeallocateRaw(void* chunk)
{
    // Push back into free list
    ChunkNode* node = reinterpret_cast<ChunkNode*>(chunk);
    node->next = freeListHead;
    freeListHead = node;
    isFull = false; // slab now has at least one free chunk
}