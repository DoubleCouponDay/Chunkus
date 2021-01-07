#pragma once

#include "types/colour.h"
#include "types/image.h"
#include "variance.h"

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