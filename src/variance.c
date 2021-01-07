#include "variance.h"
#include <math.h>
#include <stdlib.h>
#include "types/colour.h"
#include "tools.h"
#include <errno.h>

#ifndef NULL
#define NULL 0
#endif


//uses kahan summation algorithm
//void mean_of_pixels(pixel *pixels, int num_pixels, double *red, double *green, double *blue)
void mean_of_pixels(pixel *pixels, int num_pixels, pixelD* output)
{
    double sum_red, error_red, sum_green, error_green, sum_blue, error_blue;
    sum_red = error_red = sum_green = error_green = sum_blue = error_blue = 0.0;

    // Sum
    for (int i = 0; i < num_pixels; ++i)
    {
        double red_error_diff    = (double)pixels[i].r - error_red;
        double green_error_diff  = (double)pixels[i].g - error_green;
        double blue_error_diff   = (double)pixels[i].b - error_blue;
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

    if (output) {
        output->r = sum_red;
        output->g = sum_green;
        output->b = sum_blue;
    }
}


//uses shifted-data variance algorithm
node_variance calculate_pixel_variance(pixel *pixels, int num_pixels) {
    if (num_pixels < 2)
        return (node_variance){0.f, 0.f, 0.f};

    pixelD mean = {
        0.0,
        0.0,
        0.0
    };
    
    
    mean_of_pixels(pixels, num_pixels, &mean);

    double sum_red_difference = 0.f, sum_red_squared = 0.f;
    double sum_green_difference = 0.f, sum_green_squared = 0.f;
    double sum_blue_difference = 0.f, sum_blue_squared = 0.f;

    for (int i = 0; i < num_pixels; ++i)
    {
        pixelF current_colour = convert_pixel_to_colorf(pixels[i]);
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
    node_variance variance;
    variance.r = (float)((sum_red_squared - (sum_red_difference * sum_red_difference) / (double)num_pixels) / (double)(num_pixels - 1));
    variance.g = (float)((sum_green_squared - (sum_green_difference * sum_green_difference) / (double)num_pixels) / (double)(num_pixels - 1));
    variance.b = (float)((sum_blue_squared - (sum_blue_difference * sum_blue_difference) / (double)num_pixels) / (double)(num_pixels - 1));
    
    if (fabsf(variance.r) < 0.000000001)
        variance.r = 0.f;
    if (fabsf(variance.g) < 0.000000001)
        variance.g = 0.f;
    if (fabsf(variance.b) < 0.000000001)
        variance.b = 0.f;
    return variance;
}
