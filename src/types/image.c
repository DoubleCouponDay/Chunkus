#include "image.h"

#include "../tools.h"

#include <stdlib.h>

image create_image(int width, int height)
{
    image out;
    allocate_image(width, height, &out);
    return out;
}

void allocate_image(int width, int height, image *img)
{
    if (!img)
        return;

    img->width = width;
    img->height = height;

    img->pixels = malloc(sizeof(colorp) * height);

    for (int i = 0; i < width; ++i)
    {
        img->pixels[i] = malloc(sizeof(color) * width);
    }
}

void free_image(image *img)
{
    if (!img || !img->pixels)
        return;

    for (int i = 0; i < img->width; ++i)
    {
        free(img->pixels[i]);
    }

    free(img->pixels);
}