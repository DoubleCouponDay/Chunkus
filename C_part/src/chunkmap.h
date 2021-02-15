#pragma once

#include <stdlib.h>
#include "image.h"
#include <stdbool.h>

struct chunkshape; //allows circular reference

typedef struct 
{
    pixel average_colour;
    pixel** pixels_array_2d;
    coordinate location;
    struct chunkshape* shape_chunk_in;
    struct chunkshape* boundary_chunk_in;
} pixelchunk;

typedef struct pixelchunk_list
{    
    pixelchunk* chunk_p;
    struct pixelchunk_list* firstitem;
    struct pixelchunk_list* next;
} pixelchunk_list;

typedef struct chunkshape {
    bool filled;
    int chunks_amount;
    pixelchunk_list* chunks;
    int boundaries_length;
    pixelchunk_list* boundaries;
    int pathcount;
    pixel colour;
    struct chunkshape* next;
} chunkshape;

typedef struct 
{
    pixelchunk** groups_array_2d;
    chunkshape* shape_list;
    int shape_count;
    int map_width; 
    int map_height;
    image input;
} chunkmap;

typedef struct
{
    char* file_path;
    int chunk_size;
    float shape_colour_threshhold;
} vectorize_options;

chunkmap* generate_chunkmap(image inputimage_p, vectorize_options options);
void free_chunkmap(chunkmap* map_p);

int count_list(pixelchunk_list* first);
int count_shapes(chunkshape* first);