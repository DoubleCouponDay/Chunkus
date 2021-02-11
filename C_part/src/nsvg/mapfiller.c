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
#include "../hashmap/tidwall.h"
#include "../utility/error.h"
#include "copy.h"
#include "../hashmap/usage.h"
#include "mapping.h"
#include "../sort.h"

pixelchunk_list* add_chunk_to_list(chunkshape* shape, pixelchunk* chunk) {
    pixelchunk_list* new = calloc(1, sizeof(pixelchunk_list));
    new->firstitem = shape->boundaries->firstitem;
    new->chunk_p = NULL;
    new->next = NULL;

    shape->boundaries->next = new;
    ++shape->boundaries_length;
    return new;
}

chunkshape* add_new_shape(chunkshape* shape_list) {
    chunkshape* new = calloc(1, sizeof(chunkshape));

    if (!new) {
        DEBUG("allocation failed\n");        
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    hashmap* newhashy = hashmap_new(sizeof(pixelchunk), 16, 0, 0, chunk_hash, chunk_compare, NULL);

    if(newhashy == NULL) {
        DEBUG("big problem\n");
        free(new);  
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    pixelchunk_list* orderedlist = calloc(1, sizeof(pixelchunk_list));
    orderedlist->firstitem = NULL;
    orderedlist->chunk_p = NULL;
    orderedlist->next = NULL;

    new->chunks = newhashy;
    new->next = NULL;
    new->previous = shape_list;
    new->boundaries = orderedlist;
    shape_list->next = new;
    return new;
}

void add_chunk_to_hashmap(chunkshape* shape_list, pixelchunk* item) {
    if(item->shape_chunk_in != NULL) {
        return;
    }

    else if(hashmap_oom(shape_list->chunks)){
        DEBUG("hashmap out of mana\n");
        setError(HASHMAP_OOM);
        return;
    }
    void* result = hashmap_set(shape_list->chunks, item);
    item->shape_chunk_in = shape_list;
}

//returns the shape its in. else, NULL
chunkshape* big_chungus_already_in_shape(chunkmap* map, pixelchunk* chungus) {
    return chungus->shape_chunk_in;
}

typedef struct list_holder
{
    chunkshape* list;
} list_holder;

//welcome to the meat and potatoes of the program!
inline void find_shapes(chunkmap* map, pixelchunk* current, list_holder* output, chunkshape* firstshape, int map_x, int map_y, float shape_colour_threshold) {    
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
            chunkshape* chosenshape;

            if (colours_are_similar(current->average_colour, adjacent->average_colour, shape_colour_threshold)) {                
                if(currentinshape == NULL && adjacentinshape == NULL) {
                    if(firstshape->filled == false) {
                        chosenshape = firstshape;    
                    }

                    else {
                        chosenshape = output->list = add_new_shape(output->list);
                    }
                    add_chunk_to_hashmap(chosenshape, current);
                    add_chunk_to_hashmap(chosenshape, adjacent);
                    chosenshape->colour = current->average_colour;
                }

                else if (currentinshape && adjacentinshape == NULL)
                {
                    chosenshape = currentinshape;
                    add_chunk_to_hashmap(chosenshape, adjacent);
                }

                else if(currentinshape == NULL && adjacentinshape)
                {
                    chosenshape = adjacentinshape;
                    add_chunk_to_hashmap(chosenshape, current);
                }

                else {
                    continue;
                }
            }

            else { // Not similar
                if(firstshape->filled == false) { //use firstshape
                    chosenshape = firstshape;
                    chosenshape->colour = current->average_colour;
                    add_chunk_to_hashmap(chosenshape, current);
                }

                else if(currentinshape) { //set shape for boundary manipulation
                    chosenshape = currentinshape;
                }

                else { //current is not in a shape
                    chosenshape = output->list = add_new_shape(output->list);
                    chosenshape->colour = current->average_colour;
                    add_chunk_to_hashmap(chosenshape, current);
                }
                
                if(chosenshape->boundaries->chunk_p == NULL) { //use first boundary
                    chosenshape->boundaries->chunk_p = current;
                    ++chosenshape->boundaries_length;
                }

                else { //create boundary item
                    add_chunk_to_list(chosenshape, current);  
                }                
            }
        }
    }
    output->list->boundaries = output->list->boundaries->firstitem;
}

void fill_chunkmap(chunkmap* map, vectorize_options* options) {
    //create set of shapes
    DEBUG("Fill chunkmap with threshold: %f\n", options->shape_colour_threshhold);
    int tenth_of_map = (int)floorf(map->map_width * map->map_height / 10.f);
    list_holder list = (list_holder){ map->shape_list };
    int count = 0;
    int tenth_count = 0;
    chunkshape* firstshape = map->shape_list;

    for(int map_y = 0; map_y < map->map_height; ++map_y)
    {
        for(int map_x = 0; map_x < map->map_width; ++map_x)
        {
            ++count;
            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                DEBUG("Progress: %d%%\n", tenth_count);
            }
            pixelchunk* currentchunk_p = &map->groups_array_2d[map_x][map_y];
            find_shapes(map, currentchunk_p, &list, firstshape, map_x, map_y, options->shape_colour_threshhold);
            int code = getLastError();

            if (isBadError())
            {
                DEBUG("find_shapes failed with code: %d\n", code);
                return;
            }
        }
    }
}
