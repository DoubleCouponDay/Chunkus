#pragma once

#include <stdio.h>
#include <math.h>
#include "types/colour.h"
#include <nanosvg.h>

typedef struct pathshape {
    char* moveto;
    char** lineto;
};

typedef struct
{
    pixelF Stroke;
    pixelF Fill;
} svg_PropertiesF;


void vectorize_Image_Group(image input, group_map group_input, float variance_threshold);