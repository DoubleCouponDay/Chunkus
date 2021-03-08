#pragma once


#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"

void sweepfill_chunkmap(chunkmap* map, float threshold);
