#pragma once
#include "Slab.h"
constexpr int BYTE_SIZES_AMOUNT = 4;
constexpr size_t BYTE1 = 1;
constexpr size_t BYTE4 = 4;
constexpr size_t BYTE8 = 8;
constexpr size_t BYTE64 = 64;
constexpr size_t BYTE_SIZES[BYTE_SIZES_AMOUNT] = {BYTE1, BYTE4, BYTE8, BYTE64};


class MemoryManager
{
private:
	Slab* slab1;
	Slab* slab4;
	Slab* slab8;
	Slab* slab64;
	Slab* slabs[BYTE_SIZES_AMOUNT];

public:
	MemoryManager();
	template <typename T, typename... Args>
	T* CreateAndAllocate(Args&&... args);

	template <typename T>
	void DestroyAndDeallocate(T* obj);
};



// a lot to unpack here
// Args... allows for any number of types
// Args&& allows for r-values or l-values
// int x = 5;
// Foo<(x);    l-value
// Foo(42);    r-value
template<typename T, typename... Args>
inline T* MemoryManager::CreateAndAllocate(Args&&... args)
{
	size_t size = sizeof(T);
	Slab* slab = nullptr;
	for (int i = 0; i < BYTE_SIZES_AMOUNT; ++i)
	{
		size_t thisSize = BYTE_SIZES[i];
		if (size <= thisSize)
		{
			slab = slabs[i];
			break;
		}
	}

	if (slab == nullptr) return nullptr;

	void* chunk = slab->Allocate();
	// std::forward preserved "r/l-value"-ness
	return new(chunk) T(std::forward<Args>(args)...);
}

template<typename T>
inline void MemoryManager::DestroyAndDeallocate(T* obj)
{
	size_t size = sizeof(T);
	Slab* slab = nullptr;
	for (int i = 0; i < BYTE_SIZES_AMOUNT; ++i)
	{
		size_t thisSize = BYTE_SIZES[i];
		if (size <= thisSize)
		{
			slab = slabs[i];
			break;
		}
	}

	if (slab == nullptr) return;
	slab->Deallocate<T>((void*)obj);
}
