#pragma once

#include <stdio.h>
#include <math.h>

#include "../types/colour.h"
#include "../types/image.h"
#include "../mapping.h"
#include <nanosvg.h>

typedef struct
{
    chunkmap* map;
    NSVGimage* output;
    NSVGpath* first_path;
    NSVGpaint* shapescolour;
} iter_struct;

void fill_chunkmap(chunkmap* map, vectorize_options* options);

NSVGimage* vectorize_image(image input, vectorize_options options);

void free_image(NSVGimage* input);