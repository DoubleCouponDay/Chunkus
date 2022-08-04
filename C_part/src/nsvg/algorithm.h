#pragma once
#include <nanosvg.h>
#include <pthread.h>
#include "../chunkmap.h"
#include "../image.h"

typedef struct Quadrant {
    char* name;
    chunkmap* map;
    vectorize_options* options;
    square bounds;
    pthread_mutex_t* shapes_mutex;
} Quadrant;

void fill_chunkmap(chunkmap* map, vectorize_options* options);
