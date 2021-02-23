#pragma once

#include <list>
#include <memory>

#include "chunkmap.h"

bool chunk_is_adjacent(pixelchunk& current, pixelchunk& subject);
void prune_boundary(std::list<std::shared_ptr<pixelchunk>> boundary);
