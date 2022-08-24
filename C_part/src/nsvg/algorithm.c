#include "algorithm.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>
#include <pthread.h>

#include "../image.h"
#include "../chunkmap.h"
#include "../utility/logger.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"
#include "../utility/vec.h"

const float ZIP_DISTANCE = 0.5;

float get_offset(float dimension) {
    if (dimension > 0.f)
        return ZIP_DISTANCE;
        
    else if (dimension < 0.f)
        return -ZIP_DISTANCE;
    
    return 0.f;
}

void zip_border_seam(pixelchunk* current, pixelchunk* alien) {
    vector2 diff = create_vector_between_chunks(current, alien);
    current->border_location.x = current->location.x + get_offset(diff.x);
    current->border_location.y = current->location.y + get_offset(diff.y);
}

void windback_lists(chunkmap* map) {
    map->shape_list = map->first_shape;
    chunkshape* current = map->first_shape;

    while(current != NULL) {
        current->chunks = current->chunks->first_chunk;
        current->boundaries = current->boundaries->first_chunk;
        current = current->next;
    }
}

pixelchunk_list* add_chunk_to_list(chunkshape* shape, pixelchunk* chunk, pixelchunk_list* list, chunkshape** list_chunk_in, int* counter) {
    if(*list_chunk_in != NULL) {
        return list;
    }
    pixelchunk_list* new = calloc(1, sizeof(pixelchunk_list));
    new->first_chunk = list->first_chunk;
    new->chunk_p = chunk;
    new->next = NULL;

    list->next = new;
    *list_chunk_in = shape;
    ++(*counter);
    return new;
}

chunkshape* add_new_shape(Quadrant* quadrant) {
    chunkshape* new = calloc(1, sizeof(chunkshape));

    if (!new) {
        LOG_ERR("%s: allocation failed", quadrant->name);        
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    pixelchunk_list* chunks = calloc(1, sizeof(pixelchunk_list));
    chunks->first_chunk = chunks;
    chunks->chunk_p = NULL;
    chunks->next = NULL;

    pixelchunk_list* boundaries = calloc(1, sizeof(pixelchunk_list));
    boundaries->first_chunk = boundaries;
    boundaries->chunk_p = NULL;
    boundaries->next = NULL;

    new->previous = quadrant->map->shape_list;
    new->next = NULL;
    new->chunks = chunks;
    new->boundaries = boundaries;

    quadrant->map->shape_list->next = new; //links to the previous last item
    quadrant->map->shape_list = new; //sets the linked list to last item
    ++quadrant->map->shape_count;
    return new;
}

chunkshape* merge_shapes(
    Quadrant* quadrant,
    chunkshape* first, 
    chunkshape* second) {
    // Find smallest shape
    LOG_INFO("%s: merging shapes", quadrant->name);
    chunkshape* smaller = (first->chunks_amount < second->chunks_amount ? first : second);
    chunkshape* larger = (smaller == first ? second : first);

    pixelchunk_list* larger_first_chunk = larger->chunks->first_chunk;
    pixelchunk_list* larger_first_boundary = larger->boundaries->first_chunk;


    pixelchunk_list* smaller_first_chunk = smaller->chunks->first_chunk;
    pixelchunk_list* smaller_first_boundary = smaller->boundaries->first_chunk;

    int smaller_s_count = count_list(smaller->chunks->first_chunk);
    int smaller_b_count = count_list(smaller->boundaries->first_chunk);

    int larger_s_count = count_list(larger->chunks->first_chunk);
    int larger_b_count = count_list(larger->boundaries->first_chunk);

    // Replace every chunk's shape_chunk_in in second's shape holder to point to first
    if(smaller->chunks != NULL) {
        for (pixelchunk_list* iter = smaller->chunks->first_chunk; iter != NULL; iter = iter->next) {
            iter->chunk_p->shape_chunk_in = larger;
            iter->first_chunk = larger_first_chunk;
        }
    }

    if(smaller->boundaries != NULL) {
        for (pixelchunk_list* iter = smaller->boundaries->first_chunk; iter; iter = iter->next) {
            iter->chunk_p->boundary_chunk_in = larger;
            iter->first_chunk = larger_first_boundary;
        }
    }

    // Append smaller shape's chunks and borders onto larger's
    pixelchunk_list* larger_end = larger->chunks;
    
    while (larger_end->next)
        larger_end = larger_end->next;
    
    larger_end->next = smaller_first_chunk;
    larger->chunks = smaller->chunks;
    int sum = larger_s_count + smaller_s_count;
    larger->chunks_amount += smaller->chunks_amount;
    smaller->chunks = NULL;
    smaller->chunks_amount = 0;
    
    // Now append boundaries
    larger_end = larger->boundaries;

    while (larger_end->next)
        larger_end = larger_end->next;
    
    larger_end->next = smaller_first_boundary;
    larger->boundaries = smaller->boundaries;
    sum = larger_b_count + smaller_b_count;
    larger->boundaries_length += smaller->boundaries_length;
    smaller->boundaries = NULL;
    smaller->boundaries_length = 0;
    smaller->filled = false;

    // Remove smaller from the chunkshape list
    if (smaller->previous) 
    {
        if (smaller == quadrant->map->shape_list)
            quadrant->map->shape_list = smaller->previous;

        smaller->previous->next = smaller->next;

        if (smaller->next)
            smaller->next->previous = smaller->previous;
    }
    else
    { //smaller is firstshape
        quadrant->map->shape_list = smaller->next;
        quadrant->map->shape_list->previous = NULL;
    }
    --quadrant->map->shape_count;
    free(smaller);
    return larger;
}

void enlarge_border(
    Quadrant* quadrant,
    pixelchunk* current, 
    chunkshape* currentinshape, 
    chunkshape* adjacentinshape, 
    pixelchunk* adjacent) {
    chunkshape* chosenshape;
    zip_border_seam(current, adjacent);

    if(isBadError()) {
        LOG_ERR("%s: zip_border failed with code: %d", quadrant->name, getLastError());
        return;
    }

    if(quadrant->map->shape_list->filled == false) { //use firstshape
        chosenshape = quadrant->map->shape_list;
        chosenshape->filled = true;
        ++quadrant->map->shape_count;
        LOG_INFO("%s: using first shape to add a boundary to", quadrant->name);
    }

    else if(currentinshape) { //set shape for boundary manipulation
        chosenshape = currentinshape;
    }

    else { //current is not in a shape
        chosenshape = add_new_shape(quadrant);
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
            LOG_WARN("%s: A shape's chunk_p was null but chunks_amount was not", quadrant->name);
        ++chosenshape->chunks_amount;
        current->shape_chunk_in = chosenshape;
    }

    else
    {
        chosenshape->chunks = add_chunk_to_list(chosenshape, current, chosenshape->chunks, &current->shape_chunk_in, &chosenshape->chunks_amount);
    }
    chosenshape->colour = current->average_colour;
}

void enlarge_shape(
    Quadrant* quadrant,
    pixelchunk* current, 
    chunkshape* currentinshape, 
    chunkshape* adjacentinshape, 
    pixelchunk* adjacent) {
    chunkshape* chosenshape;

    if(currentinshape == NULL && adjacentinshape == NULL) {
        if(quadrant->map->shape_list->filled == false) {
            LOG_INFO("%s: Using first shape", quadrant->name);
            chosenshape = quadrant->map->shape_list;
            quadrant->map->shape_list->filled = true;
            ++quadrant->map->shape_count;
            
        }

        else {
            LOG_INFO("%s: Creating new shape", quadrant->name);
            chosenshape = add_new_shape(quadrant);
        }

        if (chosenshape->chunks->chunk_p == NULL) // If list hasn't been started, manually set the first one to current
        {
            chosenshape->chunks->chunk_p = current;
            if (chosenshape->chunks_amount)
                LOG_INFO("%s: A shape's chunk_p was null but chunks_amount was not", quadrant->name);
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
        chosenshape = merge_shapes(quadrant, currentinshape, adjacentinshape);
    }
    chosenshape->colour = current->average_colour;
    chosenshape->filled = true;
}

void find_shapes(
    Quadrant* quadrant, 
    pixelchunk* current,
    int map_x, int map_y, 
    float threshold) {    
    for (int adjacent_y = -1; adjacent_y < 2; ++adjacent_y)
    {
        for (int adjacent_x = -1; adjacent_x < 2; ++adjacent_x)
        {
            if (adjacent_x == 0 && adjacent_y == 0)
                continue; //skip center pixel
            
            int adjacent_index_x = map_x + adjacent_x;
            int adjacent_index_y = map_y + adjacent_y;

            //prevent out of bounds index
            if (adjacent_index_x < quadrant->bounds.startingX || 
                adjacent_index_y < quadrant->bounds.startingY ||
                adjacent_index_x >= quadrant->bounds.endingX ||  
                adjacent_index_y >= quadrant->bounds.endingY)
                continue;

            pixelchunk* adjacent = &(quadrant->map->groups_array_2d[adjacent_index_x][adjacent_index_y]);
            chunkshape* currentinshape = current->shape_chunk_in;
            chunkshape* adjacentinshape = adjacent->shape_chunk_in;

            if (colours_are_similar(current->average_colour, adjacent->average_colour, threshold)) {
                if(map_x == quadrant->bounds.startingX || map_x == (quadrant->bounds.endingX - 1) ||
                    map_y == quadrant->bounds.startingY || map_y == (quadrant->bounds.endingY - 1)) 
                {
                    enlarge_border(quadrant, current, currentinshape, adjacentinshape, adjacent);

                    if(isBadError()) {
                        LOG_ERR("%s enlarge_border failed with code: %d", quadrant->name, getLastError());
                        return;
                    }
                }
                enlarge_shape(quadrant, current, currentinshape, adjacentinshape, adjacent);
            }

            else {
                enlarge_border(quadrant, current, currentinshape, adjacentinshape, adjacent);

                if(isBadError()) {
                    LOG_ERR("%s enlarge_border failed with code: %d", quadrant->name, getLastError());
                    return;
                }
            }
        }
    }
}

///A multithreaded function
void* fill_quadrant(void* arg) {
    Quadrant* quadrant = (Quadrant*)arg;
    int count = 0;
    int tenth_count = 0;
    int tenth_of_map = (int)floor(quadrant->map->map_width * quadrant->map->map_height / 10.f);
    
    for(int map_y = quadrant->bounds.startingY; map_y < quadrant->bounds.endingY; ++map_y)
    {
        for(int map_x = quadrant->bounds.startingX; map_x < quadrant->bounds.endingX; ++map_x)
        {
            ++count;
            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                LOG_INFO("%s progress: %d0%%", quadrant->name, tenth_count);
            }
            pixelchunk* currentchunk_p = &(quadrant->map->groups_array_2d[map_x][map_y]);
            
            find_shapes(
                quadrant, 
                currentchunk_p,
                map_x, map_y, 
                quadrant->options->threshold);            
            
            int code = getLastError();

            if (isBadError())
            {
                LOG_ERR("%s find_shapes failed with code: %d", quadrant->name, code);
                pthread_exit(NULL);
            }

            if(quadrant->options->step_index > 0 && count >= quadrant->options->step_index) {
                LOG_INFO("step_index reached: %d\n", count);
                pthread_exit(NULL);
            }
        }
    }
    pthread_exit(NULL);
}

void fill_chunkmap(chunkmap* map, vectorize_options* options) {
    //create set of shapes
    LOG_INFO("Filling chunkmap");

    int middle_width = (int)floor((float)map->map_width / (float)2); //int divisions return 0 by default. use float division
    int middle_height = (int)floor((float)map->map_height / (float)2);

    LOG_INFO("creating quadrants");
    Quadrant quadrant1 = {"bottom-left", map, options};
    quadrant1.bounds.startingX = 0;
    quadrant1.bounds.startingY = 0;
    quadrant1.bounds.endingX = middle_width;
    quadrant1.bounds.endingY = middle_height;

    chunkmap* map2 = generate_chunkmap(map->input, *options);
    Quadrant quadrant2 = {"bottom-right", map2, options};
    quadrant2.bounds.startingX = middle_width + 1;
    quadrant2.bounds.startingY = 0;
    quadrant2.bounds.endingX = map->map_width;
    quadrant2.bounds.endingY = middle_height; 

    chunkmap* map3 = generate_chunkmap(map->input, *options);
    Quadrant quadrant3 = {"top-left", map3, options};
    quadrant3.bounds.startingX = 0;
    quadrant3.bounds.startingY = middle_height + 1;
    quadrant3.bounds.endingX = middle_width;
    quadrant3.bounds.endingY = map->map_height;

    chunkmap* map4 = generate_chunkmap(map->input, *options);
    Quadrant quadrant4 = {"top-right", map4, options};
    quadrant4.bounds.startingX = middle_width + 1;
    quadrant4.bounds.startingY = middle_height + 1;
    quadrant4.bounds.endingX = map->map_width;
    quadrant4.bounds.endingY = map->map_height;

    LOG_INFO("creating threads");
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    pthread_t thread4;
    pthread_create(&thread1, NULL, fill_quadrant, &quadrant1);
    pthread_create(&thread2, NULL, fill_quadrant, &quadrant2);
    pthread_create(&thread3, NULL, fill_quadrant, &quadrant3);
    pthread_create(&thread4, NULL, fill_quadrant, &quadrant4);
    LOG_INFO("waiting for thread1");
    pthread_join(thread1, NULL);
    LOG_INFO("waiting for thread2");
    pthread_join(thread2, NULL);
    LOG_INFO("waiting for thread3");
    pthread_join(thread3, NULL);
    LOG_INFO("waiting for thread4");
    pthread_join(thread4, NULL);
    
    LOG_INFO("winding back lists");

    windback_lists(map);
    windback_lists(map2);
    windback_lists(map3);
    windback_lists(map4);

    LOG_INFO("appending shapes from threads");

    map3->shape_list->next = map4->first_shape;
    map3->shape_count += map4->shape_count;

    map2->shape_list->next = map3->first_shape;
    map2->shape_count += map3->shape_count;

    map->shape_list->next = map2->first_shape;
    map->shape_count += map2->shape_count;
}
