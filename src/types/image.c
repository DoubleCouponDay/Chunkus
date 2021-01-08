#include "image.h"

#include "../tools.h"

#include <stdlib.h>

image create_image(int width, int height)
{
    image output = {
        width, height
    };

    DEBUG("Creating Image with %d x %d Dimensions \n", width, height);

    output.pixels_array_2d = malloc(sizeof(pixel*) * width);

    for (int i = 0; i < width; ++i)
    {
        output.pixels_array_2d[i] = malloc(sizeof(pixel) * height);
    }

    // Begin Changes
    output.topleftcorner_p     = &output.pixels_array_2d[0][0];
    output.toprightcorner_p    = &output.pixels_array_2d[output.width-1][0];
    output.bottomleftcorner_p  = &output.pixels_array_2d[0][output.height-1];
    output.bottomrightcorner_p = &output.pixels_array_2d[output.width-1][output.height-1];
    return output;
}

void free_image_contents(image img)
{
    if (!img.pixels_array_2d) {
        DEBUG("image has null pointers \n");
        return;    
    }
    
    for (int i = 0; i < img.width; ++i)
    {
        pixel* current = img.pixels_array_2d[i];

        if(current) {     
            free(current);            
        }

        else
            DEBUG("pixel is null \n");
    }

    if(img.pixels_array_2d) {
        free(img.pixels_array_2d);
    }

    else {
        DEBUG("pixel collection is null \n");
    }
}
