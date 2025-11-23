#pragma once

struct Slab
{
    typedef char Byte;

    Slab(size_t chunkSize, unsigned int chunkAmount);
    const size_t ChunkSize;
    void* start;
    void* end;
    bool isFull;

    void* Allocate();
    void DeallocateRaw(void* chunk);

    template <typename T>
    void Deallocate(void* chunk);

    ~Slab() = default;
    // internal free list node
    struct ChunkNode
    {
        ChunkNode* next;
    };

private:


    ChunkNode* freeListHead; // typed pointer for free list
};

template<typename T>
inline void Slab::Deallocate(void* chunk)
{
    reinterpret_cast<T*>(chunk)->~T();
    DeallocateRaw(chunk);
}
