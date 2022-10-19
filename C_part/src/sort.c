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

void sort_boundary_chunk(Quadrant* quadrant, chunkshape* shape, pixelchunk_list* current) {
    if(current == shape->boundaries) { //dont sort if first chunk
        return;
    }

    else if(is_adjacent(current, shape->boundaries)) { //chunk is adjacent to last and is not first
        shape->boundaries->next = current;
        shape->boundaries = current;
    }

    else if(is_adjacent(current, shape->first_boundary)) { //chunk is adjacent to first and is not last
        current->next = shape->first_boundary;
        shape->first_boundary = current;
    }
    //if no true condition found yet, something is seriously wrong
    else if(current == shape->first_boundary && shape->boundaries_length > 0) { //dont try to sort the first chunk
        LOG_ERR("current boundary chunk cannot be first! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
        return;
    }

    else if(current == shape->boundaries) { //dont try to sort the last chunk
        LOG_ERR("current boundary chunk cannot be last! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
        return;
    }

    else { //shapes boundary on next scanline starts back to front away from first or last boundary chunk
        
        LOG_ERR("current chunk is not adjacent to first or last chunk in boundary checks!");
        setError(ASSUMPTION_WRONG);
        return;
    }
}
