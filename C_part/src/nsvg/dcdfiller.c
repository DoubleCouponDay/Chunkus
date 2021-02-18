#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "usage.h"
#include "../image.h"
#include "../chunkmap.h"
#include "../../test/debug.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"

void windback_lists(chunkshape* firstshape) {
    chunkshape* current = firstshape;

    while(current != NULL) {
        current->chunks = current->chunks->firstitem;
        current->boundaries = current->boundaries->firstitem;

        current = current->next;
    }
}

pixelchunk_list* add_chunk_to_list(chunkshape* shape, pixelchunk* chunk, pixelchunk_list* list, chunkshape** list_chunk_in, int* counter) {
    if(*list_chunk_in != NULL) {
        return list;
    }
    pixelchunk_list* new = calloc(1, sizeof(pixelchunk_list));
    new->firstitem = list->firstitem;
    new->chunk_p = chunk;
    new->next = NULL;

    list->next = new;
    *list_chunk_in = shape;
    ++(*counter);
    return new;
}

chunkshape* add_new_shape(chunkmap* map, chunkshape* shape_list) {
    chunkshape* new = calloc(1, sizeof(chunkshape));

    if (!new) {
        DEBUG("allocation failed\n");        
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    pixelchunk_list* chunks = calloc(1, sizeof(pixelchunk_list));
    chunks->firstitem = chunks;
    chunks->chunk_p = NULL;
    chunks->next = NULL;

    pixelchunk_list* boundaries = calloc(1, sizeof(pixelchunk_list));
    boundaries->firstitem = boundaries;
    boundaries->chunk_p = NULL;
    boundaries->next = NULL;

    new->previous = shape_list;
    new->next = NULL;
    new->chunks = chunks;
    new->boundaries = boundaries;

    shape_list->next = new;
    ++map->shape_count;
    return new;
}

//returns the shape its in. else, NULL
chunkshape* big_chungus_already_in_shape(chunkmap* map, pixelchunk* chungus) {
    return chungus->shape_chunk_in;
}

typedef struct list_holder
{
    chunkshape* list;
} list_holder;

chunkshape* merge_shapes(chunkmap* map, list_holder* holder, chunkshape* first, chunkshape* second) {
    // Find smallest shape
    DEBUG("Merging shapes\n");
    chunkshape* smaller = (first->chunks_amount < second->chunks_amount ? first : second);
    chunkshape* larger = (smaller == first ? second : first);

    pixelchunk_list* larger_first_chunk = larger->chunks->firstitem;
    pixelchunk_list* larger_first_boundary = larger->boundaries->firstitem;


    pixelchunk_list* smaller_first_chunk = smaller->chunks->firstitem;
    pixelchunk_list* smaller_first_boundary = smaller->boundaries->firstitem;

    int smaller_s_count = count_list(smaller->chunks->firstitem);
    int smaller_b_count = count_list(smaller->boundaries->firstitem);

    int larger_s_count = count_list(larger->chunks->firstitem);
    int larger_b_count = count_list(larger->boundaries->firstitem);

    // Replace every chunk's shape_chunk_in in second's shape holder to point to first
    if(smaller->chunks != NULL) {
        for (pixelchunk_list* iter = smaller->chunks->firstitem; iter != NULL; iter = iter->next) {
            iter->chunk_p->shape_chunk_in = larger;
            iter->firstitem = larger_first_chunk;
        }
    }

    if(smaller->boundaries != NULL) {
        for (pixelchunk_list* iter = smaller->boundaries->firstitem; iter; iter = iter->next) {
            iter->chunk_p->boundary_chunk_in = larger;
            iter->firstitem = larger_first_boundary;
        }
    }

    // Append smaller shape's chunks and borders onto larger's
    pixelchunk_list* larger_end = larger->chunks;
    
    while (larger_end->next)
        larger_end = larger_end->next;
    
    larger_end->next = smaller_first_chunk;
    larger->chunks = smaller->chunks;
    int sum = larger_s_count + smaller_s_count;
    DEBUG("Larger Count: %d, Counted: %d\n", larger->chunks_amount, larger_s_count);
    DEBUG("Smaller Count: %d, Counted: %d\n", smaller->chunks_amount, smaller_s_count);
    larger->chunks_amount += smaller->chunks_amount;
    DEBUG("Sum of chunk counts: %d, larger's incremented count: %d\n", sum, larger->chunks_amount);
    DEBUG("New shape count: %d\n", count_list(larger->chunks->firstitem));
    smaller->chunks = NULL;
    smaller->chunks_amount = 0;
    
    // Now append boundaries
    larger_end = larger->boundaries;

    while (larger_end->next)
        larger_end = larger_end->next;
    
    larger_end->next = smaller_first_boundary;
    larger->boundaries = smaller->boundaries;
    sum = larger_b_count + smaller_b_count;
    DEBUG("Larger Boundaries Count: %d, Counted: %d\n", larger->boundaries_length, larger_b_count);
    DEBUG("Smaller Boundaries Count: %d, Counted: %d\n", smaller->boundaries_length, smaller_b_count);
    larger->boundaries_length += smaller->boundaries_length;
    DEBUG("Sum of boundary counts: %d, larger's incremented count: %d\n", sum, larger->boundaries_length);
    DEBUG("New boundary count: %d\n", count_list(larger->boundaries->firstitem));
    smaller->boundaries = NULL;
    smaller->boundaries_length = 0;
    smaller->filled = false;

    // Remove smaller from the chunkshape list
    if (smaller->previous) 
    {
        if (smaller == holder->list)
            holder->list = smaller->previous;

        smaller->previous->next = smaller->next;

        if (smaller->next)
            smaller->next->previous = smaller->previous;
    }
    else
    { //smaller is firstshape
        map->shape_list = smaller->next;
        map->shape_list->previous = NULL;
    }
    --map->shape_count;
    free(smaller);
    return larger;
}

void enlarge_border(chunkmap* map, pixelchunk* current, list_holder* holder, chunkshape* currentinshape, chunkshape* adjacentinshape) {
    chunkshape* chosenshape;

    if(map->shape_list->filled == false) { //use firstshape
        chosenshape = map->shape_list;
        chosenshape->filled = true;
        ++map->shape_count;
        DEBUG("Using first shape to add a boundary to\n");
    }

    else if(currentinshape) { //set shape for boundary manipulation
        chosenshape = currentinshape;
    }

    else { //current is not in a shape
        chosenshape = holder->list = add_new_shape(map, holder->list);
    }
    
    //add to boundary
    if(chosenshape->boundaries->chunk_p == NULL) { //use first boundary
        chosenshape->boundaries->chunk_p = current;
        ++chosenshape->boundaries_length;
        current->boundary_chunk_in = chosenshape;
    }

    else { //create boundary item
        chosenshape->boundaries = add_chunk_to_list(chosenshape, current, chosenshape->boundaries, &current->boundary_chunk_in, &chosenshape->boundaries_length);
    }

    //boundaries are part of the shape too
    if (chosenshape->chunks->chunk_p == NULL) { 
        chosenshape->chunks->chunk_p = current;
        if (chosenshape->chunks_amount)
            DEBUG("A shape's chunk_p was null but chunks_amount was not\n");
        ++chosenshape->chunks_amount;
        current->shape_chunk_in = chosenshape;
    }

    else
    {
        chosenshape->chunks = add_chunk_to_list(chosenshape, current, chosenshape->chunks, &current->shape_chunk_in, &chosenshape->chunks_amount);
    }
    chosenshape->colour = current->average_colour;
}

void enlarge_shape(chunkmap* map, pixelchunk* current, list_holder* holder, chunkshape* currentinshape, chunkshape* adjacentinshape, pixelchunk* adjacent) {
    chunkshape* chosenshape;

    if(currentinshape == NULL && adjacentinshape == NULL) {
        if(map->shape_list->filled == false) {
            chosenshape = map->shape_list;
            map->shape_list->filled = true;
            ++map->shape_count;
            DEBUG("Using first shape as neither current nor adjacent have shapes, and first is unfilled\n");
        }

        else {
            chosenshape = holder->list = add_new_shape(map, holder->list);
            DEBUG("Creating new shape because current and adjacent aren't in shapes\n");
        }

        if (chosenshape->chunks->chunk_p == NULL) // If list hasn't been started, manually set the first one to current
        {
            chosenshape->chunks->chunk_p = current;
            if (chosenshape->chunks_amount)
                DEBUG("A shape's chunk_p was null but chunks_amount was not\n");
            ++chosenshape->chunks_amount;
            current->shape_chunk_in = chosenshape;
        }
        else
        {
            chosenshape->chunks = add_chunk_to_list(chosenshape, current, chosenshape->chunks, &current->shape_chunk_in, &chosenshape->chunks_amount);
        }
        chosenshape->chunks = add_chunk_to_list(chosenshape, adjacent, chosenshape->chunks, &adjacent->shape_chunk_in, &chosenshape->chunks_amount);
    }

    else if (currentinshape && adjacentinshape == NULL)
    {
        chosenshape = currentinshape;
        chosenshape->chunks = add_chunk_to_list(chosenshape, adjacent, chosenshape->chunks, &adjacent->shape_chunk_in, &chosenshape->chunks_amount);
    }

    else if(currentinshape == NULL && adjacentinshape)
    {
        chosenshape = adjacentinshape;
        chosenshape->chunks = add_chunk_to_list(chosenshape, current, chosenshape->chunks, &current->shape_chunk_in, &chosenshape->chunks_amount);
    }

    else if(currentinshape == adjacentinshape) {
        chosenshape = currentinshape;
    }

    else { // Merge the two shapes        
        chosenshape = merge_shapes(map, holder, currentinshape, adjacentinshape);
    }
    chosenshape->colour = current->average_colour;
    chosenshape->filled = true;
}

//welcome to the meat and potatoes of the program!
void find_shapes(chunkmap* map, pixelchunk* current, list_holder* holder, int map_x, int map_y, float shape_colour_threshold) {    
    for (int adjacent_y = -1; adjacent_y < 2; ++adjacent_y)
    {
        for (int adjacent_x = -1; adjacent_x < 2; ++adjacent_x)
        {
            if (adjacent_x == 0 && adjacent_y == 0)
                continue; //skip center pixel
            
            int adjacent_index_x = map_x + adjacent_x;
            int adjacent_index_y = map_y + adjacent_y;

            //prevent out of bounds index
            if (adjacent_index_x < 0 || 
                adjacent_index_y < 0 ||
                adjacent_index_x >= map->map_width ||  
                adjacent_index_y >= map->map_height)
                continue;

            pixelchunk* adjacent = &map->groups_array_2d[adjacent_index_x][adjacent_index_y];
            chunkshape* currentinshape = big_chungus_already_in_shape(map, current);
            chunkshape* adjacentinshape = big_chungus_already_in_shape(map, adjacent);

            if (colours_are_similar(current->average_colour, adjacent->average_colour, shape_colour_threshold)) {
                if(map_x == 0 || map_x == (map->map_width - 1) ||
                    map_y == 0 || map_y == (map->map_height - 1)) 
                {
                    enlarge_border(map, current, holder, currentinshape, adjacentinshape);
                }
                enlarge_shape(map, current, holder, currentinshape, adjacentinshape, adjacent);
            }

            else {
                enlarge_border(map, current, holder, currentinshape, adjacentinshape);
            }
        }
    }
}

void fill_chunkmap(chunkmap* map, vectorize_options* options) {
    //create set of shapes
    DEBUG("Fill chunkmap with threshold: %f\n", options->shape_colour_threshhold);
    int tenth_of_map = (int)floorf(map->map_width * map->map_height / 10.f);
    list_holder holder = (list_holder){ map->shape_list };
    int count = 0;
    int tenth_count = 0;

    for(int map_y = 0; map_y < map->map_height; ++map_y)
    {
        for(int map_x = 0; map_x < map->map_width; ++map_x)
        {
            ++count;
            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                DEBUG("Progress: %d0%%\n", tenth_count);
            }
            pixelchunk* currentchunk_p = &map->groups_array_2d[map_x][map_y];
            find_shapes(map, currentchunk_p, &holder, map_x, map_y, options->shape_colour_threshhold);            
            int code = getLastError();

            if (isBadError())
            {
                DEBUG("find_shapes failed with code: %d\n", code);
                return;
            }
        }
    }
    windback_lists(map->shape_list);
}