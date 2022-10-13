#include "sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "chunkmap.h"
#include "utility/logger.h"
#include "utility/error.h"
#include "utility/vec.h"
#include "utility/defines.h"
#include "algorithm/algorithm.h"

pixelchunk* get_at(Quadrant* quad, int x, int y);
pixelchunk* next_boundary_chunk(Quadrant* quadrant, coordinate cur_pos, pixelchunk* prev);
void organise_chunk(pixelchunk_list* current, pixelchunk_list* next);
bool is_adjacent(pixelchunk_list* current, pixelchunk_list* other);

void sort_boundary(Quadrant* quadrant) {
    //iterate through all shapes
    //start with first chunk in boundary
    //if chunk is adjacent and shape_chunk_in is equal and boundary_chunk_in is equal and chunk is not previous boundary chunk: use it
    //set current chunk equal to the next boundary chunk and repeat until done?
    //how do we define done?
    chunkmap* map = quadrant->map;
    
    for (chunkshape* current_shape = map->first_shape; current_shape; current_shape = current_shape->next)
    {        
        pixelchunk_list* current_list = current_shape->boundaries->first;
        pixelchunk_list* previous_list = current_list;

        coordinate current_position = current_list->chunk_p->location;
        pixelchunk* next_chunk = next_boundary_chunk(quadrant, current_position, NULL);
        
        if (next_chunk == NULL)
            continue;

        current_list = next_chunk->boundary_chunk_in;

        while(current_list && current_list != current_shape->boundaries->first) {
            current_position = current_list->chunk_p->location;
            next_chunk = next_boundary_chunk(quadrant, current_position, previous_list->chunk_p);
            
            if (next_chunk == NULL)
                current_list->next = next_chunk->boundary_chunk_in;

            previous_list = current_list;
            current_list = next_chunk->boundary_chunk_in;
        }

        if(is_adjacent(current_list, current_list->first) == false) {
            LOG_ERR("boundary was sorted badly!", quadrant->name);
            setError(ASSUMPTION_WRONG);
            return;
        }
    }
}

pixelchunk* get_at(Quadrant* quad, int x, int y)
{
    if (x < quad->bounds.startingX || x >= quad->bounds.endingX ||
        y < quad->bounds.startingY || y >= quad->bounds.endingY)
    {
        return NULL;
    }

    return &quad->map->groups_array_2d[x][y];
}

pixelchunk* next_boundary_chunk(Quadrant* quadrant, coordinate cur_pos, pixelchunk* prev)
{
    coordinate next_offsets[8] = {
        (coordinate){ -1, -1 },
        (coordinate){ +0, -1 },
        (coordinate){ +1, -1 },
        (coordinate){ -1, +0 },
        (coordinate){ +1, +0 },
        (coordinate){ -1, +1 },
        (coordinate){ +0, +1 },
        (coordinate){ +1, +1 }
    };

    for (int next_i = 0; next_i < 8; ++next_i)
    {
        int neighbour_x = cur_pos.x + next_offsets[next_i].x;
        int neighbour_y = cur_pos.y + next_offsets[next_i].y;
        pixelchunk* neighbour = get_at(quadrant, neighbour_x, neighbour_y);
        if (neighbour && neighbour != prev)
        {
            return neighbour;
        }
    }
    return NULL;
}

bool is_adjacent(pixelchunk_list* current, pixelchunk_list* other) {
    int current_x = current->chunk_p->location.x;
    int current_y = current->chunk_p->location.y;
    int other_x = other->chunk_p->location.x;
    int other_y = other->chunk_p->location.y;
    int compare_x = current_x - other_x;
    int compare_y = current_y - other_y;
    return (compare_x == 1 || compare_x == -1) && (compare_y == 1 || compare_y == -1);
}
