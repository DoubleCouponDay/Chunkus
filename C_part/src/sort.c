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

pixelchunk* get_at(Quadrant* quad, int x, int y)
{
    if (x < quad->bounds.startingX || x >= quad->bounds.endingX ||
        y < quad->bounds.startingY || y >= quad->bounds.endingY)
    {
        return NULL;
    }

    return &quad->map->groups_array_2d[x][y];
}

pixelchunk* next_boundary_chunk(Quadrant* quadrant, pixelchunk* current, pixelchunk* previous)
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
        int neighbour_x = current->location.x + next_offsets[next_i].x;
        int neighbour_y = current->location.y + next_offsets[next_i].y;
        pixelchunk* neighbour = get_at(quadrant, neighbour_x, neighbour_y);

        if (neighbour &&
            neighbour != previous && //chunk was not used before
            neighbour->shape_chunk_in == current->shape_chunk_in && //chunk is in the same shape
            neighbour->boundary_chunk_in != NULL) //chunk is on the boundary
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
    bool output = (compare_x == 1 || compare_x == -1) || (compare_y == 1 || compare_y == -1);
    return output;
}


void sort_boundary(Quadrant* quadrant) {
    chunkmap* map = quadrant->map;
    
    for (chunkshape* current_shape = map->first_shape; current_shape; current_shape = current_shape->next)
    {        
        pixelchunk_list* current_list = current_shape->boundaries->first;
        pixelchunk_list* previous_list = current_list;
        pixelchunk* next_chunk = next_boundary_chunk(quadrant, current_list->chunk_p, NULL);
        
        if (next_chunk == NULL)
            continue;
        
        int counter = 0;
        current_list = next_chunk->boundary_chunk_in;

        while(current_list != NULL && current_list != current_shape->boundaries->first) {
            next_chunk = next_boundary_chunk(quadrant, current_list->chunk_p, previous_list->chunk_p);
            
            if (next_chunk != NULL && 
                next_chunk->boundary_chunk_in == current_shape->boundaries->first &&
                counter <= current_shape->boundaries_length) {
                previous_list = current_list;
                current_list = next_chunk->boundary_chunk_in;
                ++counter;
                continue;
            }
            current_list->next = NULL; //break the old border in favour of the new ending
            break;
        }

        if(current_list && is_adjacent(current_list, current_list->first) == false) {
            LOG_ERR("boundary was sorted badly!", quadrant->name);
            setError(ASSUMPTION_WRONG);
            return;
        }

        else if(current_list == NULL) {
            LOG_ERR("current_list should not be null.");
            setError(ASSUMPTION_WRONG);
            return;
        }
    }
}
