#include "variance.h"
#include <math.h>
#include <stdlib.h>
#include "types/colour.h"
#include "../test/tools.h"
#include <errno.h>

#ifndef NULL
#define NULL 0
#endif

const float TOO_SMALL = 0.000000001;


//uses kahan summation algorithm

void mean_of_pixels(pixel** pixels_array_2d, int num_pixels_x, int num_pixels_y, pixelD* output_p)
{
    double num_pixels = num_pixels_x * num_pixels_y;
    double sum_red, error_red, sum_green, error_green, sum_blue, error_blue;
    sum_red = error_red = sum_green = error_green = sum_blue = error_blue = 0.0;

    // Sum
    for (int i = 0; i < num_pixels_x; ++i)
    {
        for (int j = 0; j < num_pixels_y; ++j)
        {
            double red_error_diff    = (double)pixels_array_2d[i][j].r - error_red;
            double green_error_diff  = (double)pixels_array_2d[i][j].g - error_green;
            double blue_error_diff   = (double)pixels_array_2d[i][j].b - error_blue;
            double t_red    = sum_red + red_error_diff;
            double t_green  = sum_green + green_error_diff;
            double t_blue   = sum_blue + blue_error_diff;

            error_red   = (t_red - sum_red)     - red_error_diff;
            error_green = (t_green - sum_green) - green_error_diff;
            error_blue  = (t_blue - sum_blue)   - blue_error_diff;
        
            sum_red     = t_red;
            sum_green   = t_green;
            sum_blue    = t_blue;
        }
    }

    if (output_p) {
        output_p->r = sum_red / num_pixels;
        output_p->g = sum_green / num_pixels;
        output_p->b = sum_blue / num_pixels;
    }
}

//uses shifted-data variance algorithm
group_variance calculate_pixel_variance(pixel** pixels_array_2d, int num_pixels_x, int num_pixels_y) {
    int num_pixels = num_pixels_x * num_pixels_y;
    
    if (num_pixels < 2)
        return (group_variance){0.f, 0.f, 0.f};

    pixelD mean = {
        0.0,
        0.0,
        0.0        
    };
    mean.location = pixels_array_2d[0][0].location;
    mean_of_pixels(pixels_array_2d, num_pixels_x, num_pixels_y, &mean);

    double sum_red_difference = 0.f, sum_red_squared = 0.f;
    double sum_green_difference = 0.f, sum_green_squared = 0.f;
    double sum_blue_difference = 0.f, sum_blue_squared = 0.f;

    for (int i = 0; i < num_pixels_x; ++i)
    {
        for (int j = 0; j < num_pixels_y; ++j)
        {
            pixelF current_colour = convert_pixel_to_colorf(pixels_array_2d[i][j]);
            double red_diff     = (double)current_colour.r - mean.r;
            double green_diff   = (double)current_colour.g - mean.g;
            double blue_diff    = (double)current_colour.b - mean.b;
            sum_red_difference  += red_diff;
            sum_green_difference += green_diff;
            sum_blue_difference += blue_diff;
            sum_red_squared     += pow(red_diff, 2);
            sum_green_squared   += pow(green_diff, 2);
            sum_blue_squared    += pow(blue_diff, 2);
        }
    }
    group_variance variance;
    variance.r = (float)((sum_red_squared - (sum_red_difference * sum_red_difference) / (double)num_pixels) / (double)(num_pixels - 1));
    variance.g = (float)((sum_green_squared - (sum_green_difference * sum_green_difference) / (double)num_pixels) / (double)(num_pixels - 1));
    variance.b = (float)((sum_blue_squared - (sum_blue_difference * sum_blue_difference) / (double)num_pixels) / (double)(num_pixels - 1));
    
    if (fabsf(variance.r) < TOO_SMALL)
        variance.r = 0.f;

    if (fabsf(variance.g) < TOO_SMALL)
        variance.g = 0.f;

    if (fabsf(variance.b) < TOO_SMALL)
        variance.b = 0.f;

    return variance;
}
