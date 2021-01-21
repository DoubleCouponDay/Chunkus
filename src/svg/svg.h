#pragma once

#include <stdio.h>
#include <math.h>

#include "../types/colour.h"
#include "../types/image.h"
#include "../mapping.h"
#include <nanosvg.h>

#ifndef NSVG_RGB
#define NSVG_RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))
#endif

typedef struct
{
    chunkmap map;
    NSVGimage* output;
    NSVGpath* first_path;
    NSVGpaint* shapescolour;
} iter_struct;

NSVGimage* vectorize_image(image input, vectorize_options options);

void free_image(NSVGimage* input);