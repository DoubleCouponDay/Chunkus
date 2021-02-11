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
    chunkshape* shape_chunk_in;
} pixelchunk;

typedef struct pixelchunk_list
{    
    pixelchunk* chunk_p;
    struct pixelchunk_list* firstitem;
    struct pixelchunk_list* next;
} pixelchunk_list;

typedef struct chunkshape {
    bool filled;
    hashmap* chunks;
    int boundaries_length;
    pixelchunk_list* boundaries;
    int pathcount;
    pixel colour;
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