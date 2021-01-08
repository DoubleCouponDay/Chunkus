#pragma once

#include <stdio.h>
#include <math.h>
#include "types/colour.h"
#include <nanosvg.h>
#include "types/image.h"
#include "mapping.h"

typedef struct {
    float distance_between;
    float angle_between;
} colour_diff;

typedef struct {
    char* moveto_p;
    char** lineto_array2d;
} pathshape;

typedef struct {
    pixelF Stroke;
    pixelF Fill;
} svg_PropertiesF;

void vectorize_image(image input, groupmap output, float variance_threshold);