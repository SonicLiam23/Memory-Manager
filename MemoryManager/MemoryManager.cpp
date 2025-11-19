#include "MemoryManager.h"
#include "Slab.h"
#include "Log.h"

MemoryManager* MemoryManager::s_instance = nullptr;

Slab* MemoryManager::CreateNewSlab(size_t size, int amount)
{
	uintptr_t endOfThisSlab = reinterpret_cast<uintptr_t>((Byte*)nextPtr);
	if (endOfThisSlab > endOfMemory)
	{
		CreateNewBlock();
	}


	for (int i = 0; i < slabs.size(); ++i)
	{
		if (size <= slabs[i]->ChunkSize)
		{
			slabs.insert(slabs.begin() + i, new(nextPtr) Slab(size, amount));
			nextPtr = slabs[i]->end;
			return slabs[i];
		}
	}
	slabs.push_back(new(nextPtr) Slab(size, amount));
	nextPtr = slabs.back()->end;
	return slabs.back();
}

MemoryManager::MemoryManager()
{
	nextPtr = malloc(INIT_ALLOCATE_OVERRIDE);
	endOfMemory = reinterpret_cast<uintptr_t>((Byte*)nextPtr + INIT_ALLOCATE_OVERRIDE);
	slabs.reserve(sizeof(Slab*) * INIT_SLABS_RESERVED_OVERRIDE);

	slabs.push_back(new(nextPtr) Slab(1, 64));
	nextPtr = slabs.back()->end;

	slabs.push_back(new(nextPtr) Slab(4, 16));
	nextPtr = slabs.back()->end;

	slabs.push_back(new(nextPtr) Slab(8, 16));
	nextPtr = slabs.back()->end;

	slabs.push_back(new(nextPtr) Slab(32, 16));
	nextPtr = slabs.back()->end;
}

void MemoryManager::CreateNewBlock()
{
	nextPtr = malloc(INIT_ALLOCATE_OVERRIDE);
	endOfMemory = reinterpret_cast<uintptr_t>((Byte*)nextPtr + INIT_ALLOCATE_OVERRIDE);
}

MemoryManager* MemoryManager::Get()
{
	return (s_instance ? s_instance : s_instance = new MemoryManager());
}

void* MemoryManager::AllocateRaw(size_t size)
{
	Slab* slab = nullptr;
	size_t thisSize;
	// gets the right slab for this object size
	for (int i = 0; i < slabs.size(); ++i)
	{
		thisSize = slabs[i]->ChunkSize;
		if (size <= thisSize)
		{
			slab = slabs[i];
			break;
		}
	}

	// create a new slab that fits (bigger than the biggest)
	if (slab == nullptr)
	{
		size_t newSize = thisSize * 2;
		slab = CreateNewSlab(newSize, DEFAULT_SLAB_SIZE / newSize);

#ifdef _DEBUG
		std::stringstream logtext;
		logtext << "Could not fit data of size " << newSize << " creating a new slab to fit this size.";
		LOGTEXTM(logtext.str());
#endif
	}

	void* chunk = slab->Allocate();
	// cretae new slab of size thisSize
	if (chunk == nullptr)
	{
		slab = CreateNewSlab(thisSize, DEFAULT_SLAB_SIZE / thisSize);
		chunk = slab->Allocate();

#ifdef _DEBUG
		std::stringstream logtext;
		logtext << "Could not fit data of size " << thisSize << " due to no space left, creating a new one.";
		LOGTEXTM(logtext.str());
#endif
	}


#ifdef _DEBUG
	if (slabs.size() > INIT_SLABS_RESERVED_OVERRIDE)
	{
		std::stringstream logtext;
		logtext << "Slab space has exceeded the set value of " << INIT_SLABS_RESERVED_OVERRIDE << ". Define INIT_SLABS_RESERVED_OVERRIDE to increase this. Current amount: " << slabs.size();
		LOGTEXTM(logtext.str());
	}
#endif

	return chunk;
}

void MemoryManager::DeallocateRaw(void* chunk, size_t size)
{
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
	slab->DeallocateRaw(chunk);
}
