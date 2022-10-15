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


/**
 * @brief 
 * 
 * create linked_array_2d collection
 * the expansion capabilities of an array and the random access capabilities of an array
 * make all queries to XY() begin from the last inserted chunk
 * 
 * LinkedArray2D.XY() -> PixelChunk
 * LinkedArray2D.Next -> PixelChunk
 * LinkedArray2D.Current -> PixelChunk, the last item
 * 
 * sort boundary chunk impl
 * if current chunk is not adjacent to any other chunk and first chunk is not null, throw exception
 * if first chunk is null, set first chunk to the current chunk
 * if first chunk is not null, subtract last_x from current_x, subtract last_y from current_y
 * record the absolute value of the largest current_x, current_y
 * if current_x, current_y is negative, throw exception
 * iterate through the linkedlist2D on the X and Y axis for each increment or decrement
 * set last_x, last_y to current_x, current_y
 */ 