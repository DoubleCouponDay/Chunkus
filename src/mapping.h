#pragma once

#include "types/colour.h"
#include "types/image.h"
#include "tidwall.h"

typedef struct points {
    coordinate current;
    struct points* next;
    struct points* previous;
} points;

typedef struct 
{
    pixel average_colour;
    pixel** pixels_array_2d;
    coordinate location;
} pixelchunk;

typedef struct chunkshape {
    hashmap* chunks;
    
} chunkshape;

typedef struct 
{
    pixelchunk** groups_array_2d;
    chunkshape* shape_list;
    int map_width; 
    int map_height;
    image input_p;
} chunkmap;

typedef struct
{
    char* file_path;
    int chunk_size;
    float shape_colour_threshhold;
} vectorize_options;

chunkmap generate_chunkmap(image inputimage_p, vectorize_options options);

void free_group_map(chunkmap* map_p);