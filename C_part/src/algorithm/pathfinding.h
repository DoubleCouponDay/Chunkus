#pragma once

#include "chunkmap.h"

#include "usage.h"

typedef pixelchunk** pathfind_input;

void pathfind_shapes(Layer* layer, chunkmap* map, pathfind_input agg_input);

