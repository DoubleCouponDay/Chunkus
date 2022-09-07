#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "../chunkmap.h"

#ifdef __cplusplus
extern "C" {
#endif

bool write_svg_file(chunkmap* map, const char* filename);

extern const char* OUTPUT_PATH;

#ifdef __cplusplus
};
#endif