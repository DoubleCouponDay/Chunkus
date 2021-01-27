#pragma once

#include "mapping.h"

typedef struct colour
{
    short r, g, b;
} colour;

typedef struct nodemap
{
    struct colour* colours;
    int width;
    int height;
} nodemap;

typedef struct write_node_map_chunks_struct
{
    struct colour current;
    struct nodemap* map;
} write_node_map_chunks_struct;

image convert_png_to_image(char* fileaddress);

void write_image_to_file(image img, char* fileaddress);

void write_node_map_to_file(chunkmap map, char* fileaddress);

void write_chunkmap_to_file(chunkmap map, char* fileaddress);