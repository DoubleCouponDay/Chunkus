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
    char* moveto;
    char** lineto;
} pathshape;

typedef struct {
    pixelF Stroke;
    pixelF Fill;
} svg_PropertiesF;

void vectorize_Image_Group(image input, groupmap output, float variance_threshold);