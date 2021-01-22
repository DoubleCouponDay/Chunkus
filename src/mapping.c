#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mapping.h"
#include "types/colour.h"
#include "../test/tools.h"
#include "tidwall.h"
#include "svg/tidwallcopy.h"
#include "error.h"

void iterateImagePixels(int x, int y, image input, vectorize_options options, chunkmap output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelchunk
    pixelchunk* outputnodes = &output.groups_array_2d[x][y];
    
    // Assigned the edge case pixelchunk dimensions
    int node_width = input.width - x * options.chunk_size;
    int node_height = input.height - y * options.chunk_size;
    
    // Check if not actually on the edge
    if (node_width > options.chunk_size)
        node_width = options.chunk_size;

    if (node_height > options.chunk_size)
        node_height = options.chunk_size;
    
    outputnodes->pixels_array_2d = calloc(1, sizeof(pixel*) * node_width);
    
    for(int i = 0; i < node_width; ++i) {        
         outputnodes->pixels_array_2d[i] = calloc(1, sizeof(pixel) * node_height);
         outputnodes->pixels_array_2d[i] = &input.pixels_array_2d[x_offset + i][y_offset];
    }
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
}

chunkmap generate_chunkmap(image input, vectorize_options options)
{
    if (!input.pixels_array_2d)
    {
        DEBUG("Invalid image input \n");
        return (chunkmap){ NULL, 0, 0 };
    }

    if (input.width < 1 || input.height < 1 || !input.pixels_array_2d)
        return (chunkmap){ NULL, 0, 0 };

    if (options.chunk_size < 2)
        options.chunk_size = 2;
    
    chunkmap output;
    output.map_width = (int)ceilf((float)input.width / (float)options.chunk_size);
    output.map_height = (int)ceilf((float)input.height / (float)options.chunk_size);
    output.groups_array_2d = calloc(1, sizeof(pixelchunk*) * output.map_width);
    output.shape_list = calloc(1, sizeof(chunkshape));
    output.shape_list->chunks = hashmap_new(sizeof(chunkshape), 16, 0, 0, chunk_hash, chunk_compare, NULL);

    for (int i = 0; i < output.map_width; ++i)
    {
        output.groups_array_2d[i] = calloc(1, sizeof(pixelchunk) * output.map_height);
    }

    output.input = input;

    for (int x = 0; x < output.map_width; ++x)
    {
        for (int y = 0; y < output.map_height; ++y)
        {
            iterateImagePixels(x, y, input, options, output);
        }
    }
    return output;
}

void free_group_map(chunkmap* map_p)
{
    if (!map_p) {
        DEBUG("chunkmap is null\n");
        exit(NULL_ARGUMENT_ERROR);
    }

    for (int x = 0; x < map_p->map_width; ++x)
    {
        pixelchunk* current = map_p->groups_array_2d[x];
        free(current);
    }
    free(map_p->groups_array_2d);

    wind_back_chunkshapes(&(map_p->shape_list));

    //free all shapes
    while (map_p->shape_list)
    {
        chunkshape* next = map_p->shape_list->next;
        hashmap_free(map_p->shape_list->chunks);
        free(map_p->shape_list);
        map_p->shape_list = next;
    }
}

void wind_back_chunkshapes(chunkshape** list)
{
    if(list == NULL) {
        DEBUG("the list you passed was non existent.\n");
        exit(NULL_ARGUMENT_ERROR);
    }

    chunkshape* iter = *list;
    if (iter == NULL)
        return;

    while (iter->previous != NULL)
    {
        iter = iter->previous;
    }
    *list = iter;
}