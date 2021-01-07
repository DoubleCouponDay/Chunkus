#include "image.h"

#include "../tools.h"

#include <stdlib.h>

image create_image(int width, int height)
{
    image out;
    allocate_image(width, height, &out);
    return out;
}

void allocate_image(int width, int height, image* output_p)
{
    if (!output_p)
        return;

    output_p->width = width;
    output_p->height = height;

    output_p->pixels_array_2d = malloc(sizeof(pixel*) * width);

    for (int i = 0; i < height; ++i)
    {
        output_p->pixels_array_2d[i] = malloc(sizeof(pixel) * height);
    }

    // Begin Changes
    output_p->topleftcorner_p     = &output_p->pixels_array_2d[0]                   [0];
    output_p->toprightcorner_p    = &output_p->pixels_array_2d[output_p->width-1]   [0];
    output_p->bottomleftcorner_p  = &output_p->pixels_array_2d[0]                   [output_p->height-1];
    output_p->bottomrightcorner_p = &output_p->pixels_array_2d [output_p->width-1]  [output_p->height-1];
}

void free_image_contents(image* img_p)
{
    if (!img_p || !img_p->pixels_array_2d) {
        DEBUG("image has null pointers \n");
        return;    
    }
    
    for (int i = 0; i < img_p->width; ++i)
    {
        DEBUG("indexing pixels array\n");
        pixel* current = img_p->pixels_array_2d[i];

        if(current) {     
            DEBUG("freeing pixel\n");       
            free(current);            
        }

        else
            DEBUG("pixel is null \n");
    }

    if(img_p->pixels_array_2d) {
        DEBUG("freeing pixel collection\n");
        free(img_p->pixels_array_2d);    
    }    

    else {
        DEBUG("pixel collection is null \n");
    }
    DEBUG("freed image contents\n");
}