#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"
#include "usage.h"

void parse_map_into_nsvgimage(const chunkmap& map, nsvg_ptr& output);
