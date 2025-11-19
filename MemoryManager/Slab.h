#pragma once
#include <stack>
struct Slab
{
	typedef char Byte;
	Slab(size_t chunkSize, unsigned int chunkAmount);
	const size_t ChunkSize;
	void* start;
	void* end;
	std::stack<void*> freeChunks;
	void* Allocate();
	bool isFull;

	void DeallocateRaw(void* chunk);
	template <typename T>
	void Deallocate(void* chunk);
	

private:

};

template<typename T>
inline void Slab::Deallocate(void* chunk)
{
	DeallocateRaw(chunk);
	reinterpret_cast<T*>(chunk)->~T();
}
