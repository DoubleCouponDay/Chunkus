#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "chunkmap.h"
#include "utility/logger.h"
#include "utility/error.h"
#include "entrypoint.h"

void iterateImagePixels(int x, int y, image input, vectorize_options options, chunkmap* output) {
    int x_offset = x * options.chunk_size;
    int y_offset = y * options.chunk_size;

    // Grab the pixelchunk
    pixelchunk* chunk = &output->groups_array_2d[x][y];
    
    coordinate location = {
        x, y,
    };
    chunk->location = location;
    chunk->border_location = (vector2){ 0.f, 0.f };
    
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
        LOG_ERR("Invalid image input");
        setError(ASSUMPTION_WRONG);
        return NULL;
    }

    if (input.width < 1 || input.height < 1 || !input.pixels_array_2d)
    {
        LOG_ERR("Invalid dimensions or bad image");
        setError(ASSUMPTION_WRONG);
        return NULL;
    }    
    chunkmap* output = calloc(1, sizeof(chunkmap));
    output->input = input;
    output->map_width = (int)ceil((float)input.width / (float)options.chunk_size);
    output->map_height = (int)ceil((float)input.height / (float)options.chunk_size);
    
    LOG_INFO("creating pixelchunk");
    pixelchunk** newarray = calloc(1, sizeof(pixelchunk*) * output->map_width);
    output->groups_array_2d = newarray;
    LOG_INFO("creating chunkshape");

    chunkshape* shape_list = calloc(1, sizeof(chunkshape));
    shape_list->next = NULL;
    shape_list->previous = NULL;
    output->first_shape = shape_list;
    output->shape_list = shape_list;

    LOG_INFO("allocating boundaries list");
    pixelchunk_list* boundaries = calloc(1, sizeof(pixelchunk_list));
    boundaries->first_chunk = boundaries;
    boundaries->chunk_p = NULL;
    boundaries->next = NULL;
    shape_list->boundaries = boundaries;
        
    LOG_INFO("allocating chunks list");
    pixelchunk_list* chunks = calloc(1, sizeof(pixelchunk_list));
    chunks->first_chunk = chunks;
    chunks->chunk_p = NULL;
    chunks->next = NULL;
    shape_list->chunks = chunks;

    LOG_INFO("allocating row pointers");

    for (int i = 0; i < output->map_width; ++i)
    {
        output->groups_array_2d[i] = calloc(1, sizeof(pixelchunk) * output->map_height);
    }    
    LOG_INFO("iterating chunkmap pixels");
    
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
    LOG_INFO("freed chunkmap");
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

vector2 create_vector_between_chunks(pixelchunk* initial, pixelchunk* final) {
    int x_diff = final->location.x - initial->location.x;
    int y_diff = final->location.y - initial->location.y;
    vector2 diff = { x_diff, y_diff };
    return diff;
}

float calculate_angle_between(pixelchunk* eligible, pixelchunk* subject, pixelchunk* previous) {
    int eligible_x_diff = eligible->location.x - subject->location.x;
    int eligible_y_diff = eligible->location.y - subject->location.y;
    vector2 subject_to_eligible = { eligible_x_diff, eligible_y_diff };
    vector2 previous_to_subject;
    if (previous)
        previous_to_subject = create_vector_between_chunks(previous, subject);
    else
        previous_to_subject = subject_to_eligible;
    return vec_angle_between(previous_to_subject, subject_to_eligible);
}