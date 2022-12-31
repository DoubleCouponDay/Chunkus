#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <nanosvg.h>
#include <stdbool.h>

#include "utility/logger.h"
#include "utility/error.h"

FILE* openfile(const char* fileaddress) {
    FILE* file_p = fopen(fileaddress, "rb");

    if (!file_p) {
        LOG_ERR("Could not open file '%s' for reading", fileaddress);
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    return file_p;
}

pixel convert_colorf_to_pixel(pixelF input)
{
    pixel out;
    out.r = (byte)(input.r * 255.f);
    out.g = (byte)(input.g * 255.f);
    out.b = (byte)(input.b * 255.f);
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

colour convert_pixel_to_colour(pixel input)
{
    colour out;
    out.r = input.r;
    out.g = input.g;
    out.b = input.b;
    return out;
}

colour convert_pixelf_to_colour(pixelF input)
{
    colour out;
    out.r = (byte)(input.r * 255.f);
    out.g = (byte)(input.g * 255.f);
    out.b = (byte)(input.b * 255.f);
    return out;
}

pixelF convert_colour_to_pixelf(colour input)
{
    pixelF out;
    out.r = (float)input.r / 255.f;
    out.g = (float)input.g / 255.f;
    out.b = (float)input.b / 255.f;
    return out;
}

bool pixelf_equal(pixelF pixel1, pixelF pixel2) {
    return pixel1.r == pixel2.r && pixel1.g == pixel2.g && pixel1.b == pixel2.b;
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
    diff.r = (int)color_a.r - (int)color_b.r;
    diff.g = (int)color_a.g - (int)color_b.g;
    diff.b = (int)color_a.b - (int)color_b.b;
    float abc = sqrtf(pow(diff.r, 2) + pow(diff.g, 2) + pow(diff.b, 2));
    return abc <= max_distance; // If difference less than the threshold
}

image create_image(int width, int height)
{
    image output = {
        width, height
    };

    LOG_INFO("Creating Image with %d x %d Dimensions", width, height);

    output.pixels_array_2d = calloc(1, sizeof(pixel*) * width);

    for (int i = 0; i < width; ++i)
    {
        output.pixels_array_2d[i] = calloc(1, sizeof(pixel) * height);
    }
    return output;
}

void free_image_contents(image img)
{
    if (!img.pixels_array_2d) {
        LOG_ERR("image has null pointers", "");
        return;
    }
    
    for (int i = 0; i < img.width; ++i)
    {
        pixel* current = img.pixels_array_2d[i];

        if(current) {     
            free(current);            
        }

        else
            LOG_ERR("pixel is null", "");
    }

    if(img.pixels_array_2d) {
        free(img.pixels_array_2d);
    }

    else {
        LOG_ERR("pixel collection is null", "");
    }
}
