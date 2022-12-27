#pragma once

#include "chunkmap.h"
#include "algorithm/algorithm.h"

bool sort_boundary_chunk(Quadrant* quadrant, chunkshape* shape, pixelchunk* current);
void sort_merging_boundary(chunkshape* smaller, chunkshape* larger);
