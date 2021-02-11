#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <nanosvg.h>
#include <stdbool.h>

#include "../test/debug.h"

#include <stdlib.h>
#include "utility/error.h"

pixel convert_colorf_to_pixel(pixelF input)
{
    pixel out;
    out.r = rintf(input.r * 255.f);
    out.g = rintf(input.g * 255.f);
    out.b = rintf(input.b * 255.f);
    out.location = input.location;
    return out;
}

pixelF convert_pixel_to_colorf(pixel input)
{
    pixelF out;
    out.r = (float)input.r / 255.f;
    out.g = (float)input.g / 255.f;
    out.b = (float)input.b / 255.f;
    out.location = input.location;
    return out;
}

bool pixelf_equal(pixelF a, pixelF b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

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

    float abc = (diff.r * diff.r) + (diff.g * diff.g) + (diff.b * diff.b);
    float mag_max = max_distance * max_distance;
    return abc <= mag_max;
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
