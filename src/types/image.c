#include "image.h"

#include "../tools.h"

#include <stdlib.h>

image create_image(int width, int height)
{
    image out;
    allocate_image(width, height, &out);
    return out;
}

void allocate_image(int width, int height, image *output)
{
    if (!output)
        return;

    output->width = width;
    output->height = height;

    output->pixels = malloc(sizeof(pixel*) * height);

    for (int i = 0; i < height; ++i)
    {
        output->pixels[i] = malloc(sizeof(pixel) * width);
    }

    // Begin Changes
    output->topleftcorner = &output->pixels     [0][0];
    output->toprightcorner = &output->pixels    [output->width-1][0];
    output->bottomleftcorner = &output->pixels  [0][output->height-1];
    output->bottomrightcorner = &output->pixels [output->width-1][output->height-1];
}

void free_image_contents(image *img)
{
    if (!img || !img->pixels)
        return;

    for (int i = 0; i < img->width; ++i)
    {
        free(img->pixels[i]);
    }

    free(img->pixels);
}