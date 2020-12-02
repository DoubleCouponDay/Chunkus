#pragma once

#include "colour.h"

typedef struct
{
    color **pixels;
    int width;
    int height;
} image;

image create_image(int width, int height);

void allocate_image(int width, int height, image *img);

void free_image_contents(image *img);