#pragma once

//couldnt be named png.h due to conflict with pnglib

#include <stdlib.h>
#include <stdbool.h>
#include "../image.h"
#include "../chunkmap.h"

typedef struct {
    colour* colours;
    int width;
    int height;
} colourmap;

typedef struct {
    colourmap* map;
} png_hashies_iter;

bool file_is_png(char* fileaddress);
image convert_png_to_image(char* fileaddress);
void write_image_to_png(image img, char* fileaddres);
void write_chunkmap_to_png(chunkmap* map, char* fileaddress);
