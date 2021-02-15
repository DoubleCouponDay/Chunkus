#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "chunkmap.h"
#include "../test/debug.h"
#include "utility/error.h"

void iterateImagePixels(int x, int y, image input, vectorize_options options, chunkmap* output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelchunk
    pixelchunk* chunk = &output->groups_array_2d[x][y];
    
    int x_units = calculate_int_units(x);
    int y_units = calculate_int_units(y);

    coordinate location = {
        x, y,
        x_units, y_units
    };
    chunk->location = location;
    
    // Assigned the edge case pixelchunk dimensions
    int node_width = input.width - x * options.chunk_size;
    int node_height = input.height - y * options.chunk_size;
    
    // Check if not actually on the edge
    if (node_width > options.chunk_size)
        node_width = options.chunk_size;

    if (node_height > options.chunk_size)
        node_height = options.chunk_size;
    
    chunk->pixels_array_2d = calloc(1, sizeof(pixel*) * node_width);
    
    for(int i = 0; i < node_width; ++i) {
        chunk->pixels_array_2d[i] = &input.pixels_array_2d[x_offset + i][y_offset];
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
    chunk->average_colour = average_p;
    chunk->shape_chunk_in = NULL;
    chunk->boundary_chunk_in = NULL;
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
    
    DEBUG("creating pixelchunk\n");
    pixelchunk* newarray = calloc(1, sizeof(pixelchunk*) * output->map_width);
    output->groups_array_2d = newarray;
    DEBUG("creating chunkshape\n");

    chunkshape* shape_list = calloc(1, sizeof(chunkshape));
    shape_list->next = NULL;

    DEBUG("allocating boundaries list\n");
    pixelchunk_list* boundaries = calloc(1, sizeof(pixelchunk_list));
    boundaries->firstitem = boundaries;
    boundaries->chunk_p = NULL;
    boundaries->next = NULL;
    shape_list->boundaries = boundaries;
        
    DEBUG("allocating chunks list\n");
    pixelchunk_list* chunks = calloc(1, sizeof(pixelchunk_list));
    chunks->firstitem = chunks;
    chunks->chunk_p = NULL;
    chunks->next = NULL;
    shape_list->chunks = chunks;

    output->shape_list = shape_list;

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

void free_pixelchunklist(pixelchunk_list* linkedlist) {
    pixelchunk_list* current = linkedlist;

    while(current != NULL) {
        pixelchunk_list* next = current->next;
        free(current); //the reference held to the pixel will be cleaned up
        current = next;
    }
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
        chunkshape* current = map_p->shape_list;
        chunkshape* next;

        //free all shapes
        while (current)
        {            
            free_pixelchunklist(current->boundaries);
            free_pixelchunklist(current->chunks);
            next = current->next;
            free(current);
            current = next;
        }
    }

    if(map_p) {
        free(map_p);
    }
    DEBUG("freed chunkmap\n");
}

int count_list(pixelchunk_list* first)
{
    int count = 0;
    pixelchunk_list* iter = first;
    for (; iter; iter = iter->next)
    {
        count++;
    }
    return count;
}

int count_shapes(chunkshape* first)
{
    int count = 0;
    for (; first; first = first->next)
        ++count;
    return count;
}