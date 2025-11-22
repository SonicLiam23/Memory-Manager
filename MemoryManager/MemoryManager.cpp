#include "MemoryManager.h"

#include "Slab.h"
#include "Log.h"
#include <cmath>
#include <assert.h>

MemoryManager* MemoryManager::s_instance = nullptr;

Slab* MemoryManager::CreateNewSlab(size_t sizeofData, size_t slabSizeBytes)
{
	size_t chunkSize;
	// round slab up to the nearest power of 2, up to 64
	if (sizeofData <= 64) 
	{
		chunkSize = pow(2, ceil(log2(sizeofData)));;
	}
	// round slab up to the nearest 32
	else 
	{
		chunkSize = ((sizeofData + 31) / 32) * 32;
	}
	
	size_t chunkAmt = slabSizeBytes / chunkSize;
	slabSizeBytes += sizeof(Slab); // slab is stored in block too, account for this

	uintptr_t endOfThisSlab = reinterpret_cast<uintptr_t>((Byte*)nextPtr) + (slabSizeBytes);
	if (endOfThisSlab > endOfMemory)
	{
		CreateNewBlock();
	}

	for (int i = 0; i < slabs.size(); ++i)
	{
		if (chunkSize <= slabs[i]->ChunkSize)
		{
			slabs.insert(slabs.begin() + i, new(nextPtr) Slab(chunkSize, chunkAmt));
			assert(reinterpret_cast<uintptr_t>(nextPtr) <= endOfMemory);
			nextPtr = reinterpret_cast<void*>(endOfThisSlab);
			return slabs[i];
		}
	}
	slabs.push_back(new(nextPtr) Slab(chunkSize, chunkAmt));
	assert(reinterpret_cast<uintptr_t>(nextPtr) <= endOfMemory);
	nextPtr = reinterpret_cast<void*>(endOfThisSlab);
	return slabs.back();
}

MemoryManager::MemoryManager()
{

	nextPtr = malloc(BLOCK_SIZE_BYTES);
	endOfMemory = reinterpret_cast<uintptr_t>((Byte*)nextPtr + BLOCK_SIZE_BYTES);
	slabs.reserve(INIT_SLABS_RESERVED_OVERRIDE);
	blocks.reserve(INIT_BLOCKS_RESERVED_OVERRIDE);
	blocks.push_back(nextPtr);

	CreateNewSlab(1);
	CreateNewSlab(4);
	CreateNewSlab(8);
	CreateNewSlab(32);
	CreateNewSlab(64);
}

void MemoryManager::CreateNewBlock()
{
	LOGTEXTM("Block ran out of memory, creating a new one.");

	nextPtr = malloc(BLOCK_SIZE_BYTES);
	endOfMemory = reinterpret_cast<uintptr_t>((Byte*)nextPtr ) + BLOCK_SIZE_BYTES;
	blocks.push_back(nextPtr);
}

Slab* MemoryManager::CreateNewSlabSafe(size_t sizeofData)
{
	if (sizeofData > DEFAULT_SLAB_SIZE_BYTES)
	{
		size_t newSize = ((sizeofData + 127) / 128) * 128 * LARGE_DATA_CHUNK_AMT;
		return CreateNewSlab(sizeofData, newSize);
		LOGTEXTM("DEFAULT_SLAB_SIZE_BYTES was too small for data. increase it to reduce overhead when assigning large data types.");
	}
	else
		return CreateNewSlab(sizeofData, DEFAULT_SLAB_SIZE_BYTES);
}

MemoryManager* MemoryManager::Get()
{
	return (s_instance ? s_instance : s_instance = new MemoryManager());
}

void* MemoryManager::AllocateRaw(size_t sizeofData)
{
	Slab* slab = nullptr;
	size_t thisSize = 0;
	// gets the right slab for this object size

	for (int i = 0; i < slabs.size(); ++i)
	{
		thisSize = slabs[i]->ChunkSize;
		if (sizeofData <= thisSize)
		{
			slab = slabs[i];
			break;
		}
	}

	// create a new slab that fits (bigger than the biggest)
	if (slab == nullptr)
	{
		slab = CreateNewSlabSafe(sizeofData);

#ifdef _DEBUG
		std::stringstream logtext;
		logtext << "Could not fit data of size " << sizeofData << " creating a new slab to fit this size.";
		LOGTEXTM(logtext.str());
#endif
	}

	void* chunk = slab->Allocate();
	// cretae new slab of size thisSize
	if (chunk == nullptr)
	{
		slab = CreateNewSlabSafe(sizeofData);

		chunk = slab->Allocate();
#ifdef _DEBUG
		std::stringstream logtext;
		logtext << "Could not fit data in slab of size " << thisSize << " due to no space left, creating a new one.";
		LOGTEXTM(logtext.str());
#endif
	}


#ifdef _DEBUG
	if (slabs.size() > INIT_SLABS_RESERVED_OVERRIDE)
	{
		std::stringstream logtext;
		logtext << "Slab space has exceeded the set value of " << INIT_SLABS_RESERVED_OVERRIDE << ". Edit INIT_SLABS_RESERVED_OVERRIDE to increase this. Current amount: " << slabs.size();
		LOGTEXTM(logtext.str());
	}
#endif
	
	assert(reinterpret_cast<uintptr_t>(nextPtr) <= endOfMemory);
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

MemoryManager::~MemoryManager()
{
	for (void* ptr : blocks)
	{
		free(ptr);
	}

	for (Slab* slab : slabs)
	{
		delete slab;
	}
}
