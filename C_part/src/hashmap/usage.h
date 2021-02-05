#pragma once

#include "../chunkmap.h"

uint64_t chunk_hash(const void *chunk, uint64_t seed0, uint64_t seed1);
int chunk_compare(const void *chunk_a, const void *chunk_b, void *udata);
