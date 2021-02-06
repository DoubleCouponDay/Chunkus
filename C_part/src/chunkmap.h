#pragma once

#include <stdlib.h>
#include "image.h"
#include "hashmap/tidwall.h"
#include <stdbool.h>

typedef struct 
{
    pixel average_colour;
    pixel** pixels_array_2d;
    coordinate location;
    bool is_boundary;
} pixelchunk;

typedef struct chunkshape {
    hashmap* chunks;
    int pathcount;
    struct chunkshape* next;
    struct chunkshape* previous;
} chunkshape;

typedef struct 
{
    pixelchunk** groups_array_2d;
    chunkshape* shape_list;
    int map_width; 
    int map_height;
    image input;
    int totalpathcount;
    int totalshapecount;
} chunkmap;

typedef struct
{
    char* file_path;
    int chunk_size;
    float shape_colour_threshhold;
} vectorize_options;

chunkmap* generate_chunkmap(image inputimage_p, vectorize_options options);

void free_chunkmap(chunkmap* map_p);

void wind_back_chunkshapes(chunkshape** list);