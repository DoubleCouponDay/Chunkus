#pragma once

#include "types/colour.h"
#include "types/image.h"

typedef color node_variance;

typedef struct 
{
    color color;
    node_variance variance;
} node;

typedef struct 
{
    node *nodes;
    int width;
    int height;
} node_map;

typedef struct
{
    int chunk_size;
} node_map_options;

node_map generate_node_map(image from, node_map_options options);
color calculate_color_variance(color *colors, int num_colors);


float shifted_data_variance(float *data, int data_len);