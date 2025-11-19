#pragma once
#include "Slab.h"
#include "Log.h"
#include <vector>

#ifdef _DEBUG
#include <string>
#include <sstream>
#endif

constexpr int BYTE_SIZES_AMOUNT = 4;
constexpr size_t BYTE1 = 1;
constexpr size_t BYTE4 = 4;
constexpr size_t BYTE8 = 8;
constexpr size_t BYTE64 = 64;

#ifndef INIT_ALLOCATE_OVERRIDE
#define INIT_ALLOCATE_OVERRIDE 10000
#endif

#ifndef DEFAULT_SLAB_SIZE
#define DEFAULT_SLAB_SIZE 1000
#endif

#ifndef INIT_SLABS_RESERVED_OVERRIDE
#define INIT_SLABS_RESERVED_OVERRIDE 10
#endif // !INIT_SLABS_RESERVED_OVERRIDE


class MemoryManager
{
private:
	typedef char Byte;
	std::vector<Slab*> slabs;
	void* nextPtr = nullptr;
	uintptr_t endOfMemory;
	MemoryManager();
	void CreateNewBlock();
	Slab* CreateNewSlab(size_t size, int amount);
	
	static MemoryManager* s_instance;
	

public:
	template <typename T, typename... Args>
	T* AllocateAndCreate(Args&&... args);
	static MemoryManager* Get();
	void* AllocateRaw(size_t size);
	void DeallocateRaw(void* chunk, size_t size);


	template <typename T>
	void DestroyAndDeallocate(T* obj);
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