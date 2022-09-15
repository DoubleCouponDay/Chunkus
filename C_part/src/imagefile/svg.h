#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "../chunkmap.h"

#ifdef __cplusplus
extern "C" {
#endif

FILE* start_svg_file(int map_width, int map_height, const char* filename);
void write_svg_file(FILE* output, chunkmap* map, vectorize_options options);
void finish_svg_file(FILE* output);

extern const char* OUTPUT_PATH;

#ifdef __cplusplus
};
#endif