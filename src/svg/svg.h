#pragma once

#include <stdio.h>
#include <math.h>
#include "../types/colour.h"
#include <nanosvg.h>
#include "../types/image.h"
#include "../mapping.h"

typedef struct
{
    chunkmap map;
    NSVGimage* output;
    NSVGpath* first_path;
} iter_struct;

NSVGimage* vectorize_image(image input, vectorize_options options);

void free_image(NSVGimage* input);