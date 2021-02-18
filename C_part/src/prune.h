#pragma once

#include "chunkmap.h"

bool chunk_is_adjacent(pixelchunk* current, pixelchunk* subject);
void prune_boundary(pixelchunk_list* boundary);
