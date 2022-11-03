#pragma once
#include <nanosvg.h>
#include <pthread.h>
#include "../chunkmap.h"
#include "../image.h"

#ifndef ZIP_DIRECTION
    #define POSITIVE 1
    #define NEGATIVE -1
#endif

typedef struct Quadrant {
    char* name;
    chunkmap* map;
    vectorize_options* options;
    int X_seam;
    int Y_seam;
    square bounds;
} Quadrant;

void zip_seam(Quadrant* quadrant, pixelchunk* chunk_to_zip, pixelchunk* adjacent);
void fill_quadrant(Quadrant* quadrant);
