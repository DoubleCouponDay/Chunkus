#pragma once

#include "types/colour.h"
#include "types/image.h"
#include "variance.h"
#include "types/map.h"

typedef struct 
{
    pixel average_colour;
    node_variance variance;
    pixel* groupspixels; //todo
    coordinate location; //todo
} pixelgroup;

typedef struct 
{
    pixelgroup* nodes;
    int map_width; //todo
    int map_height; //todo
    image* input; //todo
} groupmap;

typedef struct
{
    int chunk_size;
} vectorize_options;

groupmap generate_pixel_group(image from, vectorize_options options);