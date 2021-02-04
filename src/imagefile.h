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
    struct colour colour;
    struct nodemap* map;
} write_node_map_chunks_struct;

image convert_png_to_image(char* fileaddress);

void write_image_to_bmp(image img, char* fileaddress);
void write_image_to_png(image img, char* fileaddres);
void write_chunkmap_to_png(chunkmap* map, char* fileaddress);