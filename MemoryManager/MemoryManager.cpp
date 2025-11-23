#include "MemoryManager.h"

#include "Slab.h"
#include "Log.h"
#include <cmath>
#include <assert.h>




MemoryManager* MemoryManager::s_instance = nullptr;

Slab* MemoryManager::CreateNewSlab(size_t sizeofData, size_t slabSizeBytes /*= DEFAULT_SLAB_SIZE_BYTES*/)
{
	size_t chunkSize = std::max(RoundUpToChunkSize(sizeofData), sizeof(Slab::ChunkNode));

	auto partial = partialSlabs.find(chunkSize);
	if (partial != partialSlabs.end())
	{
		// may be empty when we had some partial data, then used it
		if (!partial->second.empty())
		{
			// this slab has some free data, use it instead
			Slab* newCurrent = partial->second.back();
			partial->second.pop_back();

			currentSlabs[chunkSize] = newCurrent;
			return currentSlabs[chunkSize];
		}

	}

	// There was no partial slab to use, so we must create a new slab
	// usable bytes being the amount of "usable" data by the slab
	size_t usableBytes = (slabSizeBytes > sizeof(Slab)) ? (slabSizeBytes - sizeof(Slab)) : 0;
	size_t chunkAmt = (usableBytes / chunkSize);
	if (chunkAmt == 0) {
		// ensure at least one chunk if data is larger than DEFAULT_SLAB_SIZE_BYTES
		chunkAmt = 1;
		usableBytes = chunkAmt * chunkSize;
		slabSizeBytes = sizeof(Slab) + usableBytes;
	}
	else {
		slabSizeBytes = sizeof(Slab) + chunkAmt * chunkSize;
	}

	auto CalcEndOfSlab = [&](void* startPtr) -> uintptr_t
		{
			return reinterpret_cast<uintptr_t>((Byte*)startPtr) + slabSizeBytes;
		};

	uintptr_t endOfThisSlab = CalcEndOfSlab(nextPtr);
	if (endOfThisSlab > endOfMemory)
	{
		CreateNewBlock(slabSizeBytes);
		endOfThisSlab = CalcEndOfSlab(nextPtr);
	}

	// placement new of Slab header at nextPtr
	Slab* newSlab = new(nextPtr) Slab(chunkSize, static_cast<unsigned int>(chunkAmt));
	currentSlabs[chunkSize] = newSlab;

	nextPtr = reinterpret_cast<void*>(endOfThisSlab);



	return newSlab;
}

MemoryManager::MemoryManager()
{

	nextPtr = malloc(BLOCK_SIZE_BYTES);
	endOfMemory = reinterpret_cast<uintptr_t>((Byte*)nextPtr + BLOCK_SIZE_BYTES);
	blocks.reserve(INIT_BLOCKS_RESERVED_OVERRIDE);
	blocks.push_back(nextPtr);

	CreateNewSlab(1);
	CreateNewSlab(4);
	CreateNewSlab(8);
	CreateNewSlab(32);
	CreateNewSlab(64);
}

void MemoryManager::CreateNewBlock(size_t minSizeBytes)
{
#ifdef LOG_ALL
	LOGTEXTM("Block ran out of memory, creating a new one.");
#endif

	size_t allocSize = (minSizeBytes > BLOCK_SIZE_BYTES ? minSizeBytes : BLOCK_SIZE_BYTES);

	// Allocate the new block
	void* raw = malloc(allocSize);
	assert(raw != nullptr);

	// Align block to 16 bytes for slab safety
	uintptr_t p = reinterpret_cast<uintptr_t>(raw);
	// Align p up to the next 16-byte boundary:
	uintptr_t aligned =
		(p + 16 - 1)        // add 15 so we round *up* to the next boundary
		& ~(uintptr_t)(16 - 1); // clear the lower 4 bits (mask = ~0xF), forcing 16-byte alignment

	nextPtr = reinterpret_cast<void*>(aligned);

	// endOfMemory must be (raw + allocSize) — not (nextPtr + allocSize)!
	endOfMemory = reinterpret_cast<uintptr_t>(raw) + allocSize;

	blocks.push_back(raw);

}

Slab* MemoryManager::CreateNewSlabSafe(size_t sizeofData)
{

	if (sizeofData > DEFAULT_SLAB_SIZE_BYTES)
	{
		size_t newSize = ((sizeofData + 127) / 128) * 128 * LARGE_DATA_CHUNK_AMT;
		return CreateNewSlab(sizeofData, newSize);
#ifdef LOG_ALL
		LOGTEXTM("DEFAULT_SLAB_SIZE_BYTES was too small for data. increase it to reduce overhead when assigning large data types.");
#endif
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

	// get size
	size_t chunkSize = RoundUpToChunkSize(sizeofData);

	// try to find exact size class quickly
	auto it = currentSlabs.find(chunkSize);
	if (it != currentSlabs.end())
	{
		Slab* slab = it->second;
		if (slab)
		{
			// Find the first slab with free space, skip isFull ones
			void* chunk = slab->Allocate();
			if (slab->isFull)
			{
				auto& vec = fullSlabs[chunkSize];

				// Option A: insert sorted
				auto pos = std::lower_bound(
					vec.begin(), vec.end(),
					slab,
					[](Slab* a, Slab* b) { return a->start < b->start; }
				);
				vec.insert(pos, slab);

				// mark current slab as null
				it->second = nullptr;
			}

			if (chunk)
				return chunk;
		}

	}

#ifdef LOG_ALL
	std::stringstream logtext;
	logtext << "Could not fit data of size " << sizeofData << " creating a new slab to fit this size.";
	LOGTEXTM(logtext.str());
#endif

	// No slab had space, create a new one
	Slab* slab = CreateNewSlabSafe(sizeofData);
	void* chunk = slab->Allocate();

	return chunk;
}

void MemoryManager::DeallocateRaw(void* chunk, size_t size)
{

	// Determine the size-class we should search for:
	size_t chunkSize = RoundUpToChunkSize(size);

	// part 1
	auto& vec = partialSlabs[chunkSize];
	auto itSlab = std::upper_bound(
		vec.begin(), vec.end(),
		reinterpret_cast<uintptr_t>(chunk),
		[](uintptr_t addr, Slab* s) { return addr < reinterpret_cast<uintptr_t>(s->start); }
	);

	if (itSlab != vec.begin())
	{
		--itSlab;
		Slab* slab = *itSlab;
		if (chunk >= slab->start && chunk < slab->end)
		{
			slab->DeallocateRaw(chunk);
			return;
		}
	}


	// part 2
	auto it = fullSlabs.find(chunkSize);
	if (it != fullSlabs.end())
	{
		auto& vec = it->second;   // reference to vector

		uintptr_t c = reinterpret_cast<uintptr_t>(chunk);

		// binary search
		auto itSlab = std::upper_bound(
			vec.begin(), vec.end(),
			c,
			[](uintptr_t addr, Slab* s) {
				return addr < reinterpret_cast<uintptr_t>(s->start);
			}
		);

		if (itSlab == vec.begin())

			return; // no slab can contain this chunk

		--itSlab;

		Slab* slab = *itSlab;
		uintptr_t sStart = reinterpret_cast<uintptr_t>(slab->start);
		uintptr_t sEnd = reinterpret_cast<uintptr_t>(slab->end);

		if (c >= sStart && c < sEnd)
		{
			slab->DeallocateRaw(chunk);

			if (!slab->isFull)
			{
				// insert sorted
				auto& pVec = partialSlabs[chunkSize];
				auto pos = std::lower_bound(
					pVec.begin(), pVec.end(),
					slab,
					[](Slab* a, Slab* b) { return a->start < b->start; }
				);
				pVec.insert(pos, slab);

				// remove from fullSlabs
				*itSlab = vec.back();
				vec.pop_back();
			}
			return;
		}
	}

	// Not found: possible error (invalid pointer or wrong size provided)
#ifdef LOG_ALL
	LOGTEXTM("DeallocateRaw: chunk not found in any slab of that size class.");
#endif
}

MemoryManager::~MemoryManager()
{
	// Iterate over all size classes in the map
	for (auto& pair : partialSlabs)
	{
		auto& slabList = pair.second;
		for (Slab* slab : slabList)
		{
			// Call the destructor explicitly
			slab->~Slab();
		}
	}
	for (auto& pair : fullSlabs)
	{
		auto& slabList = pair.second;
		for (Slab* slab : slabList)
		{
			slab->~Slab();
		}
	}
	for (auto& pair : currentSlabs)
	{
		auto& slab = pair.second;
		slab->~Slab();
	}

	for (void* ptr : blocks)
	{
		free(ptr);
	}
}

void MemoryManager::Reset()
{
	if (!s_instance) return;

	// Explicitly call destructor
	s_instance->~MemoryManager();
	std::free(s_instance);

	// Clear the singleton pointer
	s_instance = nullptr;
}
