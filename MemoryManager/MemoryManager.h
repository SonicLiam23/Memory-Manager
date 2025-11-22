#pragma once
#include "Slab.h"
#include "Log.h"
#include <vector>
#include "ManagerSettings.h"

#ifdef _DEBUG
#include <string>
#include <sstream>
#endif

// chunks will either be these sizes, or round up to the nearest 32
constexpr size_t FixedChunkSizes[8] = { 1, 2, 4, 8, 16, 32, 64 };

class MemoryManager
{
private:
	typedef char Byte;
	std::vector<Slab*> slabs;
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
	Slab* slab = nullptr;
	// gets the right slab for this object size
	for (int i = 0; i < slabs.size(); ++i)
	{
		size_t thisSize = slabs[i]->ChunkSize;
		if (size <= thisSize)
		{
			slab = slabs[i];
			break;
		}
	}

	if (slab == nullptr) return;
	slab->Deallocate<T>((void*)obj);
}