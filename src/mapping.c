#include "mapping.h"
#include <math.h>
#include <stdlib.h>
#include "types/colour.h"
#include "tools.h"
#include <errno.h>

#ifndef NULL
#define NULL 0
#endif

void iterateImagePixels(int x, int y, image inputimage, node_map_options options, group_map output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelgroup
    pixelgroup *outputnodes = &output.nodes[x + y * output.width];

    // Assigned the edge case pixelgroup dimensions
    int node_width = inputimage.width - x * options.chunk_size;
    int node_height = inputimage.height - y * options.chunk_size;
    
    // Check if not actually on the edge
    if (node_width > options.chunk_size)
        node_width = options.chunk_size;
    if (node_height > options.chunk_size)
        node_height = options.chunk_size;

    int count = node_width * node_height;
    
    // Gather all the pixels into this array
    pixelF **node_data = malloc(sizeof(pixelF*) * node_width);
    
    for (int i = 0; i < node_height; ++i)
        node_data[i] = malloc(sizeof(pixelF) * node_height);

    for (int width_index = 0; width_index < node_width; ++width_index)
    {
        for (int height_index = 0; height_index < node_height; ++height_index)
        {
            int pixel_x = x * options.chunk_size + width_index;
            int pixel_y = y * options.chunk_size + height_index;

            float r = inputimage.pixels[pixel_x][pixel_y].r;
            float g = inputimage.pixels[pixel_x][pixel_y].g;
            float b = inputimage.pixels[pixel_x][pixel_y].b;

            node_data[width_index][height_index].r = r;
            node_data[width_index][height_index].g = g;
            node_data[width_index][height_index].b = b;
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
            pixel *p = &inputimage.pixels[y_offset + y][x_offset + x];
            average_r += p->r;
            average_g += p->g;
            average_b += p->b;
            if (p->r < min.r)
                min.r = p->r;
            if (p->g < min.g)
                min.g = p->g;
            if (p->b < min.b)
                min.b = p->b;
            if (p->r > max.r)
                max.r = p->r;
            if (p->g > max.g)
                max.g = p->g;
            if (p->b > max.b)
                max.b = p->b;
        }
    }

    pixel average_p = { 
        (byte)((float)average_r / (float)count), 
        (byte)((float)average_g / (float)count), 
        (byte)((float)average_b / (float)count) 
    };
    outputnodes->color = average_p;

    pixel *node_pixels = malloc(sizeof(pixel) * node_width * node_height);
    for (int x = 0; x < node_width; ++x)
    {
        for (int y = 0; y < node_height; ++y)
        {
            node_pixels[x + y * node_width] = inputimage.pixels[y_offset + y][x_offset + x];
        }
    }
    outputnodes->variance = calculate_pixel_variance(node_pixels, node_width * node_height);

    //only print 
    if ((x == y && x % 20 == 0) || (x == 0 && y == 0) || (x == (output.width - 1) && y == (output.height - 1)))
    {
        DEBUG_PRINT("pixelgroup (%d, %d) variance: (%g, %g, %g), average: (%d, %d, %d), node_width: %d, node_height %d, min: %d, %d, %d, max: %d, %d, %d\n", 
        x, y, 
        outputnodes->variance.r,
        outputnodes->variance.g,
        outputnodes->variance.b, 
        outputnodes->color.r, 
        outputnodes->color.g, 
        outputnodes->color.b, 
        node_width, 
        node_height, 
        min.r, min.g, min.b, max.r, max.g, max.b);
    }
}

group_map generate_group_map(image inputimage, node_map_options options)
{
    if (!inputimage.pixels)
    {
        DEBUG_PRINT("Invalid image input \n");
        return (group_map){ NULL, 0, 0 };
    }

    if (inputimage.width < 1 || inputimage.height < 1 || !inputimage.pixels)
        return (group_map){ NULL, 0, 0 };

    if (options.chunk_size < 2)
        options.chunk_size = 2;
    
    group_map output;
    output.width = (int)ceilf((float)inputimage.width / (float)options.chunk_size);
    output.height = (int)ceilf((float)inputimage.height / (float)options.chunk_size);

    output.nodes = malloc(sizeof(pixelgroup) * output.width * output.height);

    for (int x = 0; x < output.width; ++x)
    {
        for (int y = 0; y < output.height; ++y)
        {
            iterateImagePixels(x, y, inputimage, options, output);
        }
    }

    return output;
}
