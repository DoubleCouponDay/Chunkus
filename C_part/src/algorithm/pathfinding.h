#pragma once

#include "chunkmap.h"

/* WIP AGG INPUT - input struct may be something else */
typedef pixelchunk** pathfind_input;

void pathfind_shapes(chunkmap* map, pathfind_input agg_input);

