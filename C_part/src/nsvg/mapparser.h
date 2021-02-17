#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"

void parse_map_into_nsvgimage(chunkmap* map, NSVGimage* output);
