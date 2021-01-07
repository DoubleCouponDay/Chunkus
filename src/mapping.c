#include "mapping.h"
#include <math.h>
#include <stdlib.h>
#include "types/colour.h"
#include "tools.h"
#include <errno.h>

#ifndef NULL
#define NULL 0
#endif

void iterateImagePixels(int x, int y, image* inputimage_p, vectorize_options options, groupmap output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelgroup
    pixelgroup* outputnodes = &output.groups_array_2d[x][y];
    
    // Assigned the edge case pixelgroup dimensions
    int node_width = inputimage_p->width - x * options.chunk_size;
    int node_height = inputimage_p->height - y * options.chunk_size;
    
    // Check if not actually on the edge
    if (node_width > options.chunk_size)
        node_width = options.chunk_size;
    if (node_height > options.chunk_size)
        node_height = options.chunk_size;
    
    outputnodes->pixels_array_2d = malloc(sizeof(pixel*) * node_width);

    for (int i = 0; i < node_width; ++i)
        outputnodes->pixels_array_2d[i] = &inputimage_p->pixels_array_2d[x_offset + i][y_offset];

    int count = node_width * node_height;
    
    // Gather all the pixels into this array
    pixelF** node_data_array = malloc(sizeof(pixelF*) * node_width);
    
    for (int i = 0; i < node_height; ++i)
        node_data_array[i] = malloc(sizeof(pixelF) * node_height);

    for (int width_index = 0; width_index < node_width; ++width_index)
    {
        for (int height_index = 0; height_index < node_height; ++height_index)
        {
            int pixel_x = x * options.chunk_size + width_index;
            int pixel_y = y * options.chunk_size + height_index;

            float r = inputimage_p->pixels_array_2d[pixel_x][pixel_y].r;
            float g = inputimage_p->pixels_array_2d[pixel_x][pixel_y].g;
            float b = inputimage_p->pixels_array_2d[pixel_x][pixel_y].b;

            node_data_array[width_index][height_index].r = r;
            node_data_array[width_index][height_index].g = g;
            node_data_array[width_index][height_index].b = b;
        }
    }

    // Calculate the average of all these pixels
    pixelF average = { 0.f, 0.f, 0.f };
    int average_r = 0, average_g = 0, average_b = 0;
    
    // Also calculate the Minimum and Maximum 'colors' (values of each color)
    pixel min = { 255, 255, 255 }, max = { 0, 0, 0 };

    for (int x = 0; x < node_width; ++x)
    {
        for (int y = 0; y < node_height; ++y)
        {
            pixel* currentpixel_p = &(inputimage_p->pixels_array_2d[y_offset + y][x_offset + x]);
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
            node_pixels_array2d[x][y] = inputimage_p->pixels_array_2d[y_offset + y][x_offset + x];
        }
    }
    outputnodes->variance = calculate_pixel_variance(node_pixels_array2d, node_width, node_height);

    //only print if at the end
    if ((x == y && x % 20 == 0) || (x == 0 && y == 0) || (x == (output.map_width - 1) && y == (output.map_height - 1)))
    {
        DEBUG_PRINT("pixelgroup (%d, %d) variance: (%g, %g, %g), average: (%d, %d, %d), node_width: %d, node_height %d, min: %d, %d, %d, max: %d, %d, %d\n", 
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

groupmap generate_pixel_group(image* inputimage_p, vectorize_options options)
{
    if (!inputimage_p->pixels_array_2d)
    {
        DEBUG_PRINT("Invalid image input \n");
        return (groupmap){ NULL, 0, 0 };
    }

    if (inputimage_p->width < 1 || inputimage_p->height < 1 || !inputimage_p->pixels_array_2d)
        return (groupmap){ NULL, 0, 0 };

    if (options.chunk_size < 2)
        options.chunk_size = 2;
    
    groupmap output;
    output.map_width = (int)ceilf((float)inputimage_p->width / (float)options.chunk_size);
    output.map_height = (int)ceilf((float)inputimage_p->height / (float)options.chunk_size);
    output.groups_array_2d = malloc(sizeof(pixelgroup*) * output.map_width);

    for (int i = 0; i < output.map_width; ++i)
    {
        output.groups_array_2d[i] = malloc(sizeof(pixelgroup) * output.map_height);
    }

    output.input_p = inputimage_p;

    for (int x = 0; x < output.map_width; ++x)
    {
        for (int y = 0; y < output.map_height; ++y)
        {
            iterateImagePixels(x, y, inputimage_p, options, output);
        }
    }
    return output;
}

void free_group_map(groupmap* map_p)
{
    if (!map_p)
        return;

    for (int x = 0; x < map_p->map_width; ++x)
    {
        free(map_p->groups_array_2d[x]);
    }
    
    free(map_p->groups_array_2d);
}
