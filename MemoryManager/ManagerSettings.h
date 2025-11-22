#pragma once

constexpr size_t BLOCK_SIZE_BYTES = 16384; // default:16384
constexpr size_t DEFAULT_SLAB_SIZE_BYTES = 512; // default:512
constexpr int INIT_SLABS_RESERVED_OVERRIDE = 2000; // default:20
constexpr int INIT_BLOCKS_RESERVED_OVERRIDE = 4; // default:4
// When a datatype exceedes #DEFAULT_SLAB_SIZE_BYTES how many chunks should be made
constexpr int LARGE_DATA_CHUNK_AMT = 4; // default:4