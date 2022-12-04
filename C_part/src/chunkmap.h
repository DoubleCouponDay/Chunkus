#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "image.h"
#include "utility/vec.h"
#include "entrypoint.h"

#ifdef __cplusplus
extern "C" {
#endif

struct chunkshape; //allows circular reference

typedef struct 
{
    pixel average_colour;
    pixel** pixels_array_2d;
    coordinate location;
    vector2 border_location;
    struct chunkshape* shape_chunk_in; //used to ensure a chunk doesnt end up in multiple shapes
    struct pixelchunk_list* boundary_chunk_in;
} pixelchunk;

typedef struct pixelchunk_list
{    
    pixelchunk* chunk_p;
    struct pixelchunk_list* next;
} pixelchunk_list;

typedef struct chunkshape {
    int chunks_amount; //chunks_amount also includes boundaries_length
    int boundaries_length;
    pixelchunk_list* boundaries;
    struct pixelchunk_list* first_boundary;
    pixelchunk_list* chunks;
    struct pixelchunk_list* first_chunk;
    int pathcount;
    pixel colour;
    struct chunkshape* previous;
    struct chunkshape* next;
    bool path_closed;
} chunkshape;

typedef struct chunkmap
{
    pixelchunk** groups_array_2d;
    chunkshape* first_shape;
    chunkshape* shape_list;
    int shape_count;
    int map_width; 
    int map_height;
    image input;
} chunkmap;

int get_map_width(image input, vectorize_options options);
int get_map_height(image input, vectorize_options options);
chunkmap* generate_chunkmap(image inputimage_p, vectorize_options options);
void free_chunkmap(chunkmap* map_p);
void free_pixelchunklist(pixelchunk_list* linkedlist);
vector2 create_vector_between_chunks(pixelchunk* initial, pixelchunk* final);
float calculate_angle_between(pixelchunk* eligible, pixelchunk* subject, pixelchunk* previous);
float distance_between(pixelchunk* a, pixelchunk* b);
pixelchunk_list* create_boundaryitem(pixelchunk* chunk);

#ifdef __cplusplus
};
#endif
