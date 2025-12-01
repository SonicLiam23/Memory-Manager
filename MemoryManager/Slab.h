#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <new>
// #define CORRECT_REUSE_CHECK

struct Slab
{
    typedef char Byte;

    Slab(size_t chunkSize, unsigned int chunkAmount);

    const size_t ChunkSize;       // Size of user data
    size_t fullChunkSize;         // ChunkNode + user data, aligned
    void* start;                  // Start of first user chunk
    void* end;                    // End of last chunk
    bool isFull;
    bool inPartialList;

    struct ChunkNode
    {
        size_t dataSize;
        ChunkNode* next;
        Slab* parentSlab;
    };

    ChunkNode* freeListHead;

    Slab* nextPartial = nullptr;
    Slab* prevPartial = nullptr;

    Slab* nextFull = nullptr;
    Slab* prevFull = nullptr;

    void* Allocate();
    void DeallocateRaw(void* chunk);

    template <typename T>
    void Deallocate(void* chunk);

private:
    static uintptr_t AlignUp(uintptr_t ptr, size_t alignment)
    {
        return (ptr + alignment - 1) & ~(alignment - 1);
    }
};

template<typename T>
inline void Slab::Deallocate(void* chunk)
{
    reinterpret_cast<T*>(chunk)->~T();
    DeallocateRaw(chunk);
}
