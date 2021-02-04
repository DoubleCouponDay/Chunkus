#pragma once

#include <stdio.h>
#include <math.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"

typedef struct
{
    chunkmap* map;
    NSVGimage* output;
    NSVGpath* first_path;
    NSVGpaint* shapescolour;
} svg_hashies_iter;

void fill_chunkmap(chunkmap* map, vectorize_options* options);
NSVGimage* vectorize_image(image input, vectorize_options options);
void free_nsvg(NSVGimage* input);
