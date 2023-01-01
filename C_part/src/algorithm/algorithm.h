#pragma once
#include <nanosvg.h>
#include <pthread.h>
#include "../chunkmap.h"
#include "../image.h"

typedef struct Layer {
    int layer_index;
    chunkmap* map;
    vectorize_options* options;
} Layer;

void process_layer(Layer* layer);
