#include "Slab.h"

Slab::Slab(size_t chunkSize, unsigned int chunkAmount) : ChunkSize(chunkSize), isFull(false)
{
	
	start = (void*)((Byte*)this + sizeof(Slab));
	freeChunks.push(start);
	end = (void*)((Byte*)start + ( chunkSize * chunkAmount));
}

void* Slab::Allocate()
{
	if (isFull) return nullptr;

	void* chunk = freeChunks.top();
	freeChunks.pop();
	// if it is ever empty, we are at the "end" of used memory, essentially this:
	// 1 - used; 0 - free;
	// 1 1 1 1 0 0 0 0 0 0 0 0
	// empty after popping means we used that first 0
	if (freeChunks.empty())
	{
		void* nextChunk = (void*)((Byte*)chunk + ChunkSize);
		// check if its past the end
		uintptr_t pEnd = reinterpret_cast<uintptr_t>(end);
		uintptr_t pChunk = reinterpret_cast<uintptr_t>(nextChunk);
		if (pEnd <= pChunk)
		{
			isFull = true;
		}
		else
		{
			freeChunks.push(nextChunk);
		}
		
	}

	return chunk;
}

void Slab::DeallocateRaw(void* chunk)
{
	// convert to uintptr_t to do comparisons
	uintptr_t pStart = reinterpret_cast<uintptr_t>(start);
	uintptr_t pEnd = reinterpret_cast<uintptr_t>(end);
	uintptr_t pChunk = reinterpret_cast<uintptr_t>(chunk);
	if (pChunk < pStart || pChunk >= pEnd) return;
	freeChunks.push(chunk);
}
