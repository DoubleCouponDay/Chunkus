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
        int current_x = current->chunk_p->location.x;
        int current_y = current->chunk_p->location.y;
        int last_x = shape->boundaries->chunk_p->location.x;
        int last_y = shape->boundaries->chunk_p->location.y;
        pixelchunk_list* current_work = current;

        while(current_x <= last_x + 1) { //connect boundary chunks backwards until chunks have gone too far

            pixelchunk* top_right = get_at(quadrant, current_x + 1, current_y - 1);
            pixelchunk* right = get_at(quadrant, current_x + 1, current_y);
            pixelchunk* bottom_right = get_at(quadrant, current_x + 1, current_y + 1);
            bool top_adjacent = colours_are_similar(current_work->chunk_p->average_colour, top_right->average_colour, quadrant->options->threshold);

            current_x = current_work->chunk_p->location.x;
            current_y = current_work->chunk_p->location.y;
        }

        LOG_ERR("current chunk is not adjacent to first or last chunk in boundary checks!");
        setError(ASSUMPTION_WRONG);
        return;
    }
}
