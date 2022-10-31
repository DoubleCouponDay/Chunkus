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

typedef struct sorting_list {
    bool is_boundary;
    pixelchunk* chunk;
    float distanceto_first;
    float distanceto_last;
    struct sorting_list* next;
    bool sorted;
} sorting_list;

void prepare_list(pixelchunk* chunk) {
    if(chunk->boundary_chunk_in == NULL) {
        chunk->boundary_chunk_in = calloc(1, sizeof(pixelchunk_list));
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

bool is_boundary_chunk(Quadrant* quadrant, pixelchunk* subject) {
    int current_x = subject->location.x;
    int current_y = subject->location.y;
    pixelchunk* top_right = get_at(quadrant, current_x + 1, current_y - 1);
    pixelchunk* top = get_at(quadrant, current_x, current_y - 1);
    pixelchunk* top_left = get_at(quadrant, current_x - 1, current_y - 1);
    pixelchunk* left = get_at(quadrant, current_x - 1, current_y);
    pixelchunk* bot_left = get_at(quadrant, current_x - 1, current_y + 1);
    pixelchunk* bot = get_at(quadrant, current_x, current_y + 1);
    pixelchunk* bot_right = get_at(quadrant, current_x + 1, current_y + 1);
    pixelchunk* right = get_at(quadrant, current_x + 1, current_y);
    bool topright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(topright_dissimilar)
        return true;

    bool top_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(top_dissimilar)
        return true;

    bool topleft_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(topleft_dissimilar)
        return true;

    bool left_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(left_dissimilar)
        return true;

    bool botleft_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(botleft_dissimilar)
        return true;

    bool bot_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(bot_dissimilar)
        return true;

    bool botright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(botright_dissimilar)
        return true;

    bool right_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    return right_dissimilar;
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
        shape->boundaries->next = current; //also accounts for boundary flipping over at the second boundary item
        shape->boundaries = current;
    }

    else if(is_adjacent(current, shape->first_boundary)) { //chunk is adjacent to first and is not last
        current->next = shape->first_boundary;
        shape->first_boundary = current;
    }

    else if(current == shape->first_boundary && shape->boundaries_length > 0) { //dont try to sort the first chunk
        LOG_ERR("current boundary chunk cannot be first! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
    }

    else if(current == shape->boundaries) { //dont try to sort the last chunk
        LOG_ERR("current boundary chunk cannot be last! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
    }

    else { //shapes boundary on next scanline starts back to front away from first or last boundary chunk
        LOG_INFO("current chunk is not adjacent to first or last chunk");
        int current_x = current->chunk_p->location.x;
        int current_y = current->chunk_p->location.y;
        int last_x = shape->boundaries->chunk_p->location.x;
        int last_y = shape->boundaries->chunk_p->location.y;

        //left, top_left, top, top_right can never be on the boundary
        //bot_left, bot, bot_right, right are possible boundaries
        pixelchunk* top_right = get_at(quadrant, current_x + 1, current_y - 1);
        pixelchunk* top = get_at(quadrant, current_x, current_y - 1);
        pixelchunk* top_left = get_at(quadrant, current_x - 1, current_y - 1);
        pixelchunk* left = get_at(quadrant, current_x - 1, current_y);
        pixelchunk* bot_left = get_at(quadrant, current_x - 1, current_y + 1);
        pixelchunk* bot = get_at(quadrant, current_x, current_y + 1);
        pixelchunk* bot_right = get_at(quadrant, current_x + 1, current_y + 1);
        pixelchunk* right = get_at(quadrant, current_x + 1, current_y);

        bool topright_boundary = top_right != NULL && colours_are_similar(current->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, top_right);
        bool top_boundary = top != NULL && colours_are_similar(current->chunk_p->average_colour, top->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, top);
        bool topleft_boundary = top_left != NULL && colours_are_similar(current->chunk_p->average_colour, top_left->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, top_left);
        bool left_boundary = left != NULL && colours_are_similar(current->chunk_p->average_colour, left->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, left);
        bool botleft_boundary = bot_left != NULL && colours_are_similar(current->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, bot_left);
        bool bot_boundary = bot != NULL && colours_are_similar(current->chunk_p->average_colour, bot->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, bot);
        bool botright_boundary = bot_right != NULL && colours_are_similar(current->chunk_p->average_colour, bot_right->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, bot_right);
        bool right_boundary = right != NULL && colours_are_similar(current->chunk_p->average_colour, right->average_colour, quadrant->options->threshold) && is_boundary_chunk(quadrant, right);

        sorting_list adjacent_array[] = {
            { topright_boundary, top_right },
            { top_boundary, top },
            { topleft_boundary, top_left },
            { left_boundary, left },
            { botleft_boundary, bot_left },
            { bot_boundary, bot },
            { botright_boundary, bot_right },
            { right_boundary, right }
        };

        int num_adjacent = 0;
        sorting_list* first;
        sorting_list* temp_list;
        sorting_list* closestto_first;
        sorting_list* closestto_last;

        for(int i = 0; i < 8; ++i) {
            sorting_list current_item = adjacent_array[i];
            
            if(current_item.is_boundary) {
                num_adjacent++;

                if(num_adjacent != 1) {
                    temp_list->next = &current_item;
                    temp_list = &current_item;
                }

                else {
                    first = &current_item;
                    temp_list = &current_item;
                }
                current_item.distanceto_first = distance_between(shape->first_boundary->chunk_p, current_item.chunk);
                current_item.distanceto_last = distance_between(shape->boundaries->chunk_p, current_item.chunk);
            }
        }

        if(first == NULL) {
            LOG_ERR("no adjacents were boundary chunks!");
            setError(ASSUMPTION_WRONG);
        }

        else if(num_adjacent == 8) {
            LOG_ERR("no dissimilar chunks near current chunk!");
            setError(ASSUMPTION_WRONG);
        }
        bool all_sorted = false;

        while(all_sorted == false) {
            //iterate through linked list until all chunks are sorted
            //start from the chunk closests to last and make a path towards the chunk closest to first
        }

        // //scenario 2
        // if(bot_boundary && right_boundary) {
        //     float bot_first_distance = distance_between(bot, shape->first_boundary->chunk_p);
        //     float bot_last_distance = distance_between(bot, shape->boundaries->chunk_p);
        //     float right_first_distance = distance_between(right, shape->first_boundary->chunk_p);
        //     float right_last_distance = distance_between(right, shape->boundaries->chunk_p);

        //     prepare_list(bot);
        //     prepare_list(right);

        //     if(bot_first_distance <= right_first_distance) {
        //         current->next = bot->boundary_chunk_in;
        //         right->boundary_chunk_in->next = current;
        //     }

        //     else {
        //         current->next = right->boundary_chunk_in;
        //         bot->boundary_chunk_in = current;
        //     }
        // }

    }
}
