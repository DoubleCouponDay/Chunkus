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

    else { //dont add to boundary if they are not adjacent
        LOG_ERR("current chunk is not adjacent to first or last chunk in boundary checks!");
        setError(ASSUMPTION_WRONG);
        return;
    }
}
