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

typedef struct sorting_item {
    pixelchunk* chunk;
    int num_dissimilar;
    bool is_boundary;
} sorting_item;

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

/// @brief returns the info about the first dissimilar chunk
/// @param quadrant 
/// @param subject 
/// @return 
sorting_item is_boundary_chunk(Quadrant* quadrant, pixelchunk* subject) {
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
    int num_dissimilar = 0;
    sorting_item output = {NULL, 0, false};

    bool topright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(topright_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }
        

    bool top_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(top_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    bool topleft_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(topleft_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    bool left_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(left_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    bool botleft_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(botleft_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    bool bot_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(bot_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    bool botright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(botright_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    bool right_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(right_dissimilar) {
        ++num_dissimilar;
        output.num_dissimilar = num_dissimilar;
    }

    return output;
}

bool is_adjacent(pixelchunk* current, pixelchunk_list* other) {
    int current_x = current->location.x;
    int current_y = current->location.y;
    int other_x = other->chunk_p->location.x;
    int other_y = other->chunk_p->location.y;
    int compare_x = current_x - other_x;
    int compare_y = current_y - other_y;
    bool output = (compare_x == 1 || compare_x == -1) || (compare_y == 1 || compare_y == -1);
    return output;
}

/// @brief Assume current->shape_chunk_in is NULL
/// @param quadrant 
/// @param shape 
/// @param current 
void sort_boundary_chunk(Quadrant* quadrant, chunkshape* shape, pixelchunk* current) {
    if(current == shape->boundaries->chunk_p) { //dont sort if first chunk
        LOG_ERR("current boundary chunk cannot be last! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
        return;
    }

    else if(current == shape->first_boundary->chunk_p && shape->boundaries_length > 0) { //dont try to sort the first chunk
        LOG_ERR("current boundary chunk cannot be first! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
        return;
    }

    else if(current == shape->boundaries->chunk_p) { //dont try to sort the last chunk
        LOG_ERR("current boundary chunk cannot be last! this is a finite linked list.");
        setError(ASSUMPTION_WRONG);
        return;
    }

    else if(is_adjacent(current, shape->boundaries)) { //chunk is adjacent to last and is not first
        pixelchunk_list* list = create_boundaryitem(current);
        shape->boundaries->next = list; //also accounts for boundary flipping over at the second boundary item
        shape->boundaries = list;
    }

    else if(is_adjacent(current, shape->first_boundary)) { //chunk is adjacent to first and is not last
        pixelchunk_list* list = create_boundaryitem(current);
        list->next = shape->first_boundary;
        shape->first_boundary = list;
    }

    else { //shapes boundary on next scanline starts back to front away from first or last boundary chunk
        LOG_INFO("current chunk is not adjacent to first or last chunk");
        pixelchunk_list* sort_focus = shape->boundaries;

        while(sort_focus != NULL) {
            int current_x = sort_focus->chunk_p->location.x;
            int current_y = sort_focus->chunk_p->location.y;
            int last_x = shape->boundaries->chunk_p->location.x;
            int last_y = shape->boundaries->chunk_p->location.y;

            pixelchunk* top_right = get_at(quadrant, current_x + 1, current_y - 1);
            pixelchunk* top = get_at(quadrant, current_x, current_y - 1);
            pixelchunk* top_left = get_at(quadrant, current_x - 1, current_y - 1);
            pixelchunk* left = get_at(quadrant, current_x - 1, current_y);
            pixelchunk* bot_left = get_at(quadrant, current_x - 1, current_y + 1);
            pixelchunk* bot = get_at(quadrant, current_x, current_y + 1);
            pixelchunk* bot_right = get_at(quadrant, current_x + 1, current_y + 1);
            pixelchunk* right = get_at(quadrant, current_x + 1, current_y);

            //if chunk is inside the quadrant and similar to current chunk and adjacent to dissimilar chunk and 
            sorting_item topright_ = is_boundary_chunk(quadrant, top_right);
            bool topright_boundary = top_right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && topright_diff != 0 && top_right->boundary_chunk_in == NULL;
            int top_diff = is_boundary_chunk(quadrant, top);
            bool top_boundary = top != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top->average_colour, quadrant->options->threshold) && top_diff != 0 && top->boundary_chunk_in == NULL;
            int topleft_diff = is_boundary_chunk(quadrant, top_left);
            bool topleft_boundary = top_left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top_left->average_colour, quadrant->options->threshold) && topleft_diff != 0 && top_left->boundary_chunk_in == NULL;
            int left_diff = is_boundary_chunk(quadrant, left);
            bool left_boundary = left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, left->average_colour, quadrant->options->threshold) && left_diff != 0 && left->boundary_chunk_in == NULL;
            int botleft_diff = is_boundary_chunk(quadrant, bot_left);
            bool botleft_boundary = bot_left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && botleft_diff != 0 && bot_left->boundary_chunk_in == NULL;
            int bot_diff = is_boundary_chunk(quadrant, bot);
            bool bot_boundary = bot != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot->average_colour, quadrant->options->threshold) && bot_diff != 0 && bot->boundary_chunk_in == NULL;
            int botright_diff = is_boundary_chunk(quadrant, bot_right);
            bool botright_boundary = bot_right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_right->average_colour, quadrant->options->threshold) && botright_diff != 0 && bot_right->boundary_chunk_in == NULL;
            int right_diff  = is_boundary_chunk(quadrant, right);
            bool right_boundary = right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, right->average_colour, quadrant->options->threshold) && right_diff != 0 && right->boundary_chunk_in == NULL;

            sorting_item adjacent_array[] = {
                { top_right, topright_diff, topright_boundary },
                { top, top_diff, top_boundary },
                { top_left, topleft_diff, topleft_boundary },
                { left, left_diff, left_boundary },
                { bot_left, botleft_diff, botleft_boundary },
                { bot, bot_diff, bot_boundary },
                { bot_right, botright_diff, botright_boundary },
                { right, right_diff, right_boundary }
            };

            int num_have_adjacent = 0;
            pixelchunk* highest = NULL;
            int highest_adjacent = 0;

            for(int i = 0; i < 8; ++i) {
                sorting_item current_item = adjacent_array[i];
                
                if(current_item.num_dissimilar != 0) {
                    ++num_have_adjacent;
                    
                    if(current_item.num_dissimilar > highest_adjacent) {
                        highest_adjacent = current_item.num_dissimilar;
                        highest = adjacent_array[i].chunk;
                    }
                }
            }

            if(num_have_adjacent == 0) {
                sort_focus->next = NULL;
                sort_focus = NULL;
                return;
            }

            else if(num_have_adjacent == 8) {
                LOG_ERR("no dissimilar chunks near current chunk!");
                setError(ASSUMPTION_WRONG);
                return;
            }
            pixelchunk_list* list = create_boundaryitem(highest);
            shape->boundaries->next = list;
            shape->boundaries = list;
            sort_focus = list;
            zip_seam(quadrant, highest, );
        }
    }
}
