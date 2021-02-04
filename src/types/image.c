#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <nanosvg.h>
#include <stdbool.h>

#include "../../test/tools.h"

#include <stdlib.h>
#include "../error.h"

int calculate_int_units(int subject) {
    int sum = 1;

    while(log(sum) > 1) {
        sum *= 10;
    }
    return sum;
}

bool colours_are_similar(pixel color_a, pixel color_b, float max_distance)
{
    pixel diff;
    diff.r = color_a.r - color_b.r;
    diff.g = color_a.g - color_b.g;
    diff.b = color_a.b - color_b.b;

    float mag = sqrt(pow(diff.r, 2) + pow(diff.g, 2) + pow(diff.b, 2)); //pythagorean theorem

    return mag <= max_distance;
}

char* rgb_to_string(pixel* input) {
    char* output = input->r;
    output += ',';
    output += input->g;
    output += ',';
    output += input->b;
    return output;
}

image create_image(int width, int height)
{
    image output = {
        width, height
    };

    DEBUG("Creating Image with %d x %d Dimensions \n", width, height);

    output.pixels_array_2d = calloc(1, sizeof(pixel*) * width);

    for (int i = 0; i < width; ++i)
    {
        output.pixels_array_2d[i] = calloc(1, sizeof(pixel) * height);
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
