#pragma once
#include <stack>
struct Slab
{
	typedef char Byte;
	Slab(size_t chunkSize, unsigned int chunkAmount);
	size_t _chunkSize;
	void* start;
	void* end;
	std::stack<void*> freeChunks;
	void* Allocate();

	template <typename T>
	void Deallocate(void* chunk);
};

template<typename T>
inline void Slab::Deallocate(void* chunk)
{
	// convert to uintptr_t to do comparisons
	uintptr_t pStart = reinterpret_cast<uintptr_t>(start);
	uintptr_t pEnd = reinterpret_cast<uintptr_t>(end);
	uintptr_t pChunk = reinterpret_cast<uintptr_t>(chunk);
	if (pChunk < pStart || pChunk > pEnd) return;
	freeChunks.push(chunk);
	reinterpret_cast<T*>(chunk)->~T();
}
