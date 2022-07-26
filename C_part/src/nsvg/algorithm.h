#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"

typedef struct quadrant_arguments {
    chunkmap* map;
    vectorize_options* options;
    int startingX;
    int startingY;
    int endingX;
    int endingY;
} quadrant_arguments;

void fill_chunkmap(chunkmap* map, vectorize_options* options);
