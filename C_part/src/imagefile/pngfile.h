#pragma once

//couldnt be named png.h due to conflict with pnglib

#include <stdlib.h>
#include "image.h"
#include "chunkmap.h"

image convert_png_to_image(char* fileaddress);
void write_image_to_png(const image& img, char* fileaddres);
void write_chunkmap_to_png(const chunkmap& map, char* fileaddress);
