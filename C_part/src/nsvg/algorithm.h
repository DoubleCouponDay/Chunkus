#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"
#include "../image.h"

typedef struct {
    chunkmap* map;
    vectorize_options* options;
    Rectangle bounds;
} Quadrant;

void fill_chunkmap(chunkmap* map, vectorize_options* options);
