#pragma once


#include <nanosvg.h>

#include "image.h"
#include "chunkmap.h"

typedef struct find_shapes_speed_stuff
{
    int* shape_ints;
    int* shape_counts;
    int* shape_offsets;
    int num_shapes;
    int* chunk_index_of;
    int* border_counts;
    int* border_offsets;
    int* border_chunk_indices;
} find_shapes_speed_stuff;

find_shapes_speed_stuff produce_shape_stuff(chunkmap* map, float threshold);
void shape_initial_sweep(chunkmap* map, find_shapes_speed_stuff* stuff, float threshold);
void shape_aggregate_sweep(chunkmap* map, find_shapes_speed_stuff* stuff, float threshold);

NSVGimage* vectorize_image_speed(image input, vectorize_options options);
