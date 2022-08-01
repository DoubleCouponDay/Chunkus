#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"
#include "../image.h"

typedef struct {
    chunkmap* map;
    vectorize_options* options;
    square bounds;
} Quadrant;

void fill_chunkmap(chunkmap* map, vectorize_options* options);
