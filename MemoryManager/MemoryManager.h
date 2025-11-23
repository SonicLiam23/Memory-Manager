#pragma once
#include "Slab.h"
#include "Log.h"
#include <vector>
#include <unordered_map>
#include "ManagerSettings.h"
#define TIME_FUNC

#ifdef _DEBUG
#include <string>
#include <sstream>

#ifdef TIME_FUNC
#include <chrono>
#endif

#endif



// chunks will either be these sizes, or round up to the nearest 32
constexpr size_t FixedChunkSizes[8] = { 1, 2, 4, 8, 16, 32, 64 };

class MemoryManager
{
private:
	typedef char Byte;
	std::unordered_map<size_t, Slab*> currentSlabs;
	std::unordered_map<size_t, std::vector<Slab*>> partialSlabs;
	std::unordered_map<size_t, std::vector<Slab*>> fullSlabs;
	std::vector<void*> blocks;

	void* nextPtr = nullptr;
	uintptr_t endOfMemory;
	MemoryManager();
	void CreateNewBlock(size_t minSizeBytes);
	// "safe" as you give it the size of the data, it spits out a slab, simple :D
	Slab* CreateNewSlabSafe(size_t sizeofData);
	// not "safe" because you need to get the size correct for your data, use 
	Slab* CreateNewSlab(size_t sizeofData, size_t slabSizeBytes = DEFAULT_SLAB_SIZE_BYTES);
	
	static MemoryManager* s_instance;
	

public:
	template <typename T, typename... Args>
	T* AllocateAndCreate(Args&&... args);
	static MemoryManager* Get();
	void* AllocateRaw(size_t size);
	void DeallocateRaw(void* chunk, size_t size);


	template <typename T>
	void DestroyAndDeallocate(T* obj);

	~MemoryManager();
	void Reset();
};



// a lot to unpack here
// Args... allows for any number of types
// Args&& allows for r-values or l-values
// int x = 5;
// Foo(x);    l-value
// Foo(42);   r-value
template<typename T, typename... Args>
inline T* MemoryManager::AllocateAndCreate(Args&&... args)
{
	size_t size = sizeof(T);

	void* chunk = AllocateRaw(size);

	// std::forward preserved "r/l-value"-ness
	return new(chunk) T(std::forward<Args>(args)...);
}

template<typename T>
inline void MemoryManager::DestroyAndDeallocate(T* obj)
{
	const size_t size = sizeof(T);
	size_t chunkSize = RoundUpToChunkSize(size);

	auto it = currentSlabs.find(chunkSize);
	if (it == currentSlabs.end()) return;

	uintptr_t c = reinterpret_cast<uintptr_t>(obj);
	for (Slab* slab : it->second)
	{
		uintptr_t sStart = reinterpret_cast<uintptr_t>(slab->start);
		uintptr_t sEnd = reinterpret_cast<uintptr_t>(slab->end);
		if (c >= sStart && c < sEnd)
		{
			// call destructor then free
			obj->~T();
			slab->DeallocateRaw(obj);
			return;
		}
	}

#ifdef LOG_ALL
	LOGTEXTM("DestroyAndDeallocate: object not found in any slab for that size.");
#endif
}

// Round up to size class
static inline size_t RoundUpToChunkSize(size_t size)
{
	if (size == 0) return 1;

	if (size <= 64) {
		// next power of two
		size_t v = size - 1;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
#if INTPTR_MAX > INT32_MAX
		v |= v >> 32;
#endif
		return v + 1;
	}
	else {
		// round up to nearest multiple of 128
		return ((size + 127) / 128) * 128;
	}
}