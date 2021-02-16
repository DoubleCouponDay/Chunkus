#pragma once


#include <nanosvg.h>

#include "image.h"
#include "chunkmap.h"

NSVGimage* sweepfill_chunkmap(chunkmap* map, float threshold);
