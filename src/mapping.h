#pragma once

#include "types/colour.h"
#include "types/image.h"
#include "variance.h"



typedef struct 
{
    pixel average_colour;
    node_variance variance;
    pixel** pixels_array_2d;
    coordinate location;
} pixelgroup;

typedef struct 
{
    pixelgroup** groups_array_2d;
    int map_width; 
    int map_height;
    image input_p;
} groupmap;

typedef struct
{
    int chunk_size;
} vectorize_options;

groupmap generate_pixel_group(image inputimage_p, vectorize_options options);

void free_group_map(groupmap map_p);