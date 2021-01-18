#pragma once

#include <stdio.h>
#include <math.h>
#include "../types/colour.h"
#include <nanosvg.h>
#include "../types/image.h"
#include "../mapping.h"

typedef struct {
    float distance_between;
    float angle_between;
} colour_diff;

NSVGimage* vectorize_image(image input, chunkmap output, float shape_colour_threshhold);

void free_image(NSVGimage* input);