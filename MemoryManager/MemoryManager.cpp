#include "MemoryManager.h"
#include "Slab.h"

MemoryManager::MemoryManager()
{
	slab1 = new Slab(BYTE1, 64);
	slabs[0] = slab1;

	slab4 = new Slab(BYTE4, 16);
	slabs[1] = slab4;

	slab8 = new Slab(BYTE8, 16);
	slabs[2] = slab8;

	slab64 = new Slab(BYTE64, 16);
	slabs[3] = slab64;
}
