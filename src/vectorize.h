#pragma once

#include "types/colour.h"
#include "types/image.h"

typedef colorf node_variance;

typedef struct 
{
    pixel color;
    node_variance variance;
} pixelgroup;

typedef struct 
{
    pixelgroup *nodes;
    int width;
    int height;
} group_map;

typedef struct
{
    int chunk_size;
} node_map_options;

group_map generate_group_map(image from, node_map_options options);
colorf calculate_pixel_variance(pixel *colors, int num_colors);


float shifted_data_variance(float *data, int data_len);