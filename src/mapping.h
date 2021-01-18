#pragma once

#include "types/colour.h"
#include "types/image.h"

typedef struct {
    coordinate current;
    points* next;
    points* previous;
} points;

typedef struct 
{
    pixel average_colour;
    pixel** pixels_array_2d;
    coordinate location;
} pixelchunk;

typedef struct {
    void* stuff;
} chunkshape;

typedef struct 
{
    pixelchunk** groups_array_2d;
    int map_width; 
    int map_height;
    image input_p;
} chunkmap;

typedef struct
{
    char* file_path;
    int chunk_size;
    float boundary_variance_threshold;
    float shape_colour_threshhold;
} vectorize_options;

chunkmap generate_pixel_group(image inputimage_p, vectorize_options options);

void free_group_map(chunkmap* map_p);