#pragma once

#include <stdio.h>
#include <math.h>
#include <nanosvg.h>
#include <pthread.h>

#include "../image.h"
#include "../chunkmap.h"
#include "./algorithm.h"

typedef struct LayerOperation {
    pthread_t thread;
    Layer layer;
} LayerOperation;

void vectorize(image input, vectorize_options options);
void free_nsvg(NSVGimage* input);

