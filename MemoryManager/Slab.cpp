#include "Slab.h"

Slab::Slab(size_t chunkSize, unsigned int chunkAmount) : _chunkSize(chunkSize)
{
	
	// run in global scope, just incase of an accidental class overload
	start = ::operator new(chunkSize * chunkAmount);
	freeChunks.push(start);
	end = (void*)((Byte*)start + chunkAmount);
}

void* Slab::Allocate()
{
	void* chunk = freeChunks.top();
	freeChunks.pop();
	// if it is ever empty, we are at the "end" of used memory, essentially this:
	// 1 - used; 0 - free;
	// 1 1 1 1 0 0 0 0 0 0 0 0
	// empty after popping means we used that first 0
	if (freeChunks.empty())
	{
		void* nextChunk = (void*)((Byte*)chunk + _chunkSize);
		freeChunks.push(nextChunk);
	}

	return chunk;
}