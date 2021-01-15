#include "mapping.h"
#include <math.h>
#include <stdlib.h>
#include "types/colour.h"
#include "../test/tools.h"
#include <errno.h>

#ifndef NULL
#define NULL 0
#endif

void iterateImagePixels(int x, int y, image input, vectorize_options options, groupmap output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelgroup
    pixelgroup* outputnodes = &output.groups_array_2d[x][y];
    
    // Assigned the edge case pixelgroup dimensions
    int node_width = input.width - x * options.chunk_size;
    int node_height = input.height - y * options.chunk_size;
    
    // Check if not actually on the edge
    if (node_width > options.chunk_size)
        node_width = options.chunk_size;
    if (node_height > options.chunk_size)
        node_height = options.chunk_size;
    
    outputnodes->pixels_array_2d = malloc(sizeof(pixel*) * node_width);

    for (int i = 0; i < node_width; ++i)
        outputnodes->pixels_array_2d[i] = &input.pixels_array_2d[x_offset + i][y_offset];

    int count = node_width * node_height;

    // Calculate the average of all these pixels
    pixelF average = { 0.f, 0.f, 0.f };
    int average_r = 0, average_g = 0, average_b = 0;
    
    // Also calculate the Minimum and Maximum 'colors' (values of each color)
    pixel min = { 255, 255, 255 }, max = { 0, 0, 0 };

    for (int x = 0; x < node_width; ++x)
    {
        for (int y = 0; y < node_height; ++y)
        {
            pixel* currentpixel_p = &(input.pixels_array_2d[y_offset + y][x_offset + x]);
            average_r += currentpixel_p->r;
            average_g += currentpixel_p->g;
            average_b += currentpixel_p->b;

            if (currentpixel_p->r < min.r)
                min.r = currentpixel_p->r;

            if (currentpixel_p->g < min.g)
                min.g = currentpixel_p->g;

            if (currentpixel_p->b < min.b)
                min.b = currentpixel_p->b;

            if (currentpixel_p->r > max.r)
                max.r = currentpixel_p->r;

            if (currentpixel_p->g > max.g)
                max.g = currentpixel_p->g;
                
            if (currentpixel_p->b > max.b)
                max.b = currentpixel_p->b;
        }
    }

    pixel average_p = { 
        (byte)((float)average_r / (float)count), 
        (byte)((float)average_g / (float)count), 
        (byte)((float)average_b / (float)count) 
    };
    outputnodes->average_colour = average_p;

    //malloc space for columns
    pixel** node_pixels_array2d = malloc(sizeof(pixel*) * node_width);

    //malloc space for rows
    for (int i = 0; i < node_height; ++i)
        node_pixels_array2d[i] = malloc(sizeof(pixel) * node_height);

    for (int x = 0; x < node_width; ++x)
    {
        for (int y = 0; y < node_height; ++y)
        {
            node_pixels_array2d[x][y] = input.pixels_array_2d[y_offset + y][x_offset + x];
        }
    }
    outputnodes->variance = calculate_pixel_variance(node_pixels_array2d, node_width, node_height);

    //only print if at the end
    if ((x == y && x % 20 == 0) || (x == 0 && y == 0) || (x == (output.map_width - 1) && y == (output.map_height - 1)))
    {
        DEBUG("pixelgroup (%d, %d) variance: (%g, %g, %g), average: (%d, %d, %d), node_width: %d, node_height %d, min: %d, %d, %d, max: %d, %d, %d \n", 
        x, y, 
        outputnodes->variance.r,
        outputnodes->variance.g,
        outputnodes->variance.b, 
        outputnodes->average_colour.r, 
        outputnodes->average_colour.g, 
        outputnodes->average_colour.b, 
        node_width, 
        node_height, 
        min.r, min.g, min.b, max.r, max.g, max.b);
    }
}

groupmap generate_pixel_group(image input, vectorize_options options)
{
    if (!input.pixels_array_2d)
    {
        DEBUG("Invalid image input \n");
        return (groupmap){ NULL, 0, 0 };
    }

    if (input.width < 1 || input.height < 1 || !input.pixels_array_2d)
        return (groupmap){ NULL, 0, 0 };

    if (options.chunk_size < 2)
        options.chunk_size = 2;
    
    groupmap output;
    output.map_width = (int)ceilf((float)input.width / (float)options.chunk_size);
    output.map_height = (int)ceilf((float)input.height / (float)options.chunk_size);
    output.groups_array_2d = malloc(sizeof(pixelgroup*) * output.map_width);

    for (int i = 0; i < output.map_width; ++i)
    {
        output.groups_array_2d[i] = malloc(sizeof(pixelgroup) * output.map_height);
    }

    output.input_p = input;

    for (int x = 0; x < output.map_width; ++x)
    {
        for (int y = 0; y < output.map_height; ++y)
        {
            iterateImagePixels(x, y, input, options, output);
        }
    }
    return output;
}

void free_group_map(groupmap* map_p)
{
    if (!map_p) {
        DEBUG("groupmap is null\n");
        return;
    }
    DEBUG("freeing groups\n");

    for (int x = 0; x < map_p->map_width; ++x)
    {
        pixelgroup* current = map_p->groups_array_2d[x];
        free(current);
    }
    free(map_p->groups_array_2d);
}
