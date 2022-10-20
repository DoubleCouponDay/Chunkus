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

    /**
     * problem
	I am trying to sort boundary chunks immediately after they get flagged as a boundary but it is not working

    guess
        the boundary chunks are sometimes not adjacent to the first or last boundary chunks
        
    try
        build the boundary backwards until it hits the last chunk
        
        left, top left, top, top right chunks can never be on the boundary
        
        try to find two adajacent chunks that are boundaries
        
        if bot left, bot, bot right, right are possible boundaries
        
        assume there to be at most 3 boundaries
        
        if less than 2 found, throw error
        
        if 2 or 3 found
            if this is the first loop, save the chunk furthest away from the last chunk for later
            
            if 2 found
                set furthest away chunk to next
                
                set closest chunks next to current chunk
                
            if 3 found
                set furthest away's chunk's next to second furthest away
                
                set second furthest away chunk to next
                
                set closest chunk's next to current
                
        loop until connected with adjacent

        any chunk added to the boundary should be immediately zipped
        
        disable zipping in algorithm.c if sort.c sort_boundary_chunk returned true

     * 
     */
        LOG_INFO("%s: building boundary backwards from current: %dx,%dy to last: %dx:%dy", quadrant->name, current->chunk_p->location.x, current->chunk_p->location.y, shape->boundaries->chunk_p->location.x, shape->boundaries->chunk_p->location.y);
        int current_x = current->chunk_p->location.x;
        int current_y = current->chunk_p->location.y;
        int last_x = shape->boundaries->chunk_p->location.x;
        int last_y = shape->boundaries->chunk_p->location.y;
        pixelchunk_list* current_work = current;

        while(current_x <= last_x + 2) { //connect boundary chunks backwards until chunks have gone too far
            pixelchunk* bot_left = get_at(quadrant, current_x - 1, current_y + 1);
            pixelchunk* bot = get_at(quadrant, current_x, current_y + 1);
            pixelchunk* bot_right = get_at(quadrant, current_x + 1, current_y + 1);            
            pixelchunk* right = get_at(quadrant, current_x + 1, current_y);
            pixelchunk* top_right = get_at(quadrant, current_x + 1, current_y - 1);
 
            bool top_similar = colours_are_similar(current_work->chunk_p->average_colour, top_right->average_colour, quadrant->options->threshold);
            bool mid_similar = colours_are_similar(current_work->chunk_p->average_colour, right->average_colour, quadrant->options->threshold);
            bool bot_similar = colours_are_similar(current_work->chunk_p->average_colour, bot_right->average_colour, quadrant->options->threshold);

            
            current_x = current_work->chunk_p->location.x;
            current_y = current_work->chunk_p->location.y;
        }

        LOG_ERR("current chunk is not adjacent to first or last chunk in boundary checks!");
        setError(ASSUMPTION_WRONG);
        return;
    }
}
