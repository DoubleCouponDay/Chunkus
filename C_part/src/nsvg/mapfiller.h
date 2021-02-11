#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"

void fill_chunkmap(chunkmap* map, vectorize_options* options);
void iterate_chunk_shapes(chunkmap* map, NSVGimage* output);