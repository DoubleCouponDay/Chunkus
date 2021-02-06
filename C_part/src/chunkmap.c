#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "chunkmap.h"
#include "../test/debug.h"
#include "hashmap/tidwall.h"
#include "hashmap/usage.h"
#include "utility/error.h"

void iterateImagePixels(int x, int y, image input, vectorize_options options, chunkmap* output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelchunk
    pixelchunk* outputnodes = &output->groups_array_2d[x][y];
    
    int x_units = calculate_int_units(x);
    int y_units = calculate_int_units(y);

    coordinate location = {
        x, y,
        x_units, y_units
    };
    outputnodes->location = location;
    
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
            pixel* currentpixel_p = &(input.pixels_array_2d[x_offset + x][y_offset + y]);
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

chunkmap* generate_chunkmap(image input, vectorize_options options)
{
    if (!input.pixels_array_2d)
    {
        DEBUG("Invalid image input \n");
        setError(ASSUMPTION_WRONG);
        return NULL;
    }

    if (input.width < 1 || input.height < 1 || !input.pixels_array_2d)
    {
        DEBUG("Invalid dimensions or bad image\n");
        setError(ASSUMPTION_WRONG);
        return NULL;
    }    
    chunkmap* output = calloc(1, sizeof(chunkmap));
    output->input = input;
    output->map_width = (int)ceilf((float)input.width / (float)options.chunk_size);
    output->map_height = (int)ceilf((float)input.height / (float)options.chunk_size);
    output->totalpathcount = 0;
    output->totalshapecount = 0;
    
    DEBUG("creating pixelchunk\n");
    pixelchunk* newarray = calloc(1, sizeof(pixelchunk*) * output->map_width);
    output->groups_array_2d = newarray;
    DEBUG("creating chunkshape\n");
    chunkshape* shape_list = calloc(1, sizeof(chunkshape));
    shape_list->next = NULL;
    shape_list->previous = NULL;
    output->shape_list = shape_list;
    DEBUG("allocating new hashmap\n");
    hashmap* newhashy = hashmap_new(sizeof(pixelchunk), 16, 0, 0, chunk_hash, chunk_compare, NULL); 

    if(newhashy == NULL) {
        DEBUG("new hashmap failed during creation\n");
        setError(ASSUMPTION_WRONG);
        return NULL;
    }

    DEBUG("assign shape_list hashmap\n");
    output->shape_list->chunks = newhashy;

    DEBUG("allocating row pointers\n");

    for (int i = 0; i < output->map_width; ++i)
    {
        output->groups_array_2d[i] = calloc(1, sizeof(pixelchunk) * output->map_height);
    }    
    DEBUG("iterating chunkmap pixels\n");
    
    for (int x = 0; x < output->map_width; ++x)
    {
        for (int y = 0; y < output->map_height; ++y)
        {
            iterateImagePixels(x, y, input, options, output);
        }
    }
    return output;
}

//takes a double pointer so that we can update the list itself
void wind_back_chunkshapes(chunkshape** list)
{
    if(list == NULL) {
        DEBUG("the list you passed was non existent.\n");
        return;
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

void free_chunkmap(chunkmap* map_p)
{
    if (!map_p) {
        return;
    }

    else {
        for (int x = 0; x < map_p->map_width; ++x)
        {
            pixelchunk* current = map_p->groups_array_2d[x];
            free(current);
        }
    }

    if(map_p->groups_array_2d) {
        free(map_p->groups_array_2d);
    }
    
    if(map_p->shape_list) {
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

    if(map_p) {
        free(map_p);
    }
}
