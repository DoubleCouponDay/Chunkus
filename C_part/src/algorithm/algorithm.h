#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"
#include "../image.h"

typedef struct Layer {
    int layer_index;
    chunkmap* map;
    vectorize_options* options;
    chunkshape* boundaries;
    chunkshape* non_boundaries;
} Layer;

void zip_seam(pixelchunk* chunk_to_zip, pixelchunk* adjacent);
void process_layer(Layer* layer);
