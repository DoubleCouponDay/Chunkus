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
    pixelchunk* chunk;
    int num_dissimilar;
    bool is_boundary;
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

/// @brief returns the number of adjacent dissimilar chunks
/// @param quadrant 
/// @param subject 
/// @return 
int is_boundary_chunk(Quadrant* quadrant, pixelchunk* subject) {
    if(subject == 0) {
        return 0;
    }
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
    int output = 0;

    bool topright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(topright_dissimilar)
        ++output;

    bool top_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(top_dissimilar)
        ++output;

    bool topleft_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(topleft_dissimilar)
        ++output;

    bool left_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(left_dissimilar)
        ++output;

    bool botleft_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(botleft_dissimilar)
        ++output;

    bool bot_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(bot_dissimilar)
        ++output;

    bool botright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;
    
    if(botright_dissimilar)
        ++output;

    bool right_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, quadrant->options->threshold) == false;

    if(right_dissimilar)
        ++output;

    return output;
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
        pixelchunk_list* sort_focus = shape->boundaries;

        while(sort_focus != NULL) {
            int current_x = current->chunk_p->location.x;
            int current_y = current->chunk_p->location.y;
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
            int topright_diff = is_boundary_chunk(quadrant, top_right);
            bool topright_boundary = top_right != NULL && colours_are_similar(current->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && topright_diff != 0 && top_right->boundary_chunk_in == NULL;
            int top_diff = is_boundary_chunk(quadrant, top);
            bool top_boundary = top != NULL && colours_are_similar(current->chunk_p->average_colour, top->average_colour, quadrant->options->threshold) && top_diff != 0 && top->boundary_chunk_in == NULL;
            int topleft_diff = is_boundary_chunk(quadrant, top_left);
            bool topleft_boundary = top_left != NULL && colours_are_similar(current->chunk_p->average_colour, top_left->average_colour, quadrant->options->threshold) && topleft_diff != 0 && top_left->boundary_chunk_in == NULL;
            int left_diff = is_boundary_chunk(quadrant, left);
            bool left_boundary = left != NULL && colours_are_similar(current->chunk_p->average_colour, left->average_colour, quadrant->options->threshold) && left_diff != 0 && left->boundary_chunk_in == NULL;
            int botleft_diff = is_boundary_chunk(quadrant, bot_left);
            bool botleft_boundary = bot_left != NULL && colours_are_similar(current->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && botleft_diff != 0 && bot_left->boundary_chunk_in == NULL;
            int bot_diff = is_boundary_chunk(quadrant, bot);
            bool bot_boundary = bot != NULL && colours_are_similar(current->chunk_p->average_colour, bot->average_colour, quadrant->options->threshold) && bot_diff != 0 && bot->boundary_chunk_in == NULL;
            int botright_diff = is_boundary_chunk(quadrant, bot_right);
            bool botright_boundary = bot_right != NULL && colours_are_similar(current->chunk_p->average_colour, bot_right->average_colour, quadrant->options->threshold) && botright_diff != 0 && bot_right->boundary_chunk_in == NULL;
            int right_diff  = is_boundary_chunk(quadrant, right);
            bool right_boundary = right != NULL && colours_are_similar(current->chunk_p->average_colour, right->average_colour, quadrant->options->threshold) && right_diff != 0 && right->boundary_chunk_in == NULL;

            sorting_list adjacent_array[] = {
                { top_right, topright_diff, topright_boundary },
                { top, top_diff, top_boundary },
                { top_left, topleft_diff, topleft_boundary },
                { left, left_diff, left_boundary },
                { bot_left, botleft_diff, botleft_boundary },
                { bot, bot_diff, bot_boundary },
                { bot_right, botright_diff, botright_boundary },
                { right, right_diff, right_boundary }
            };

            int num_adjacent = 0;
            sorting_list* first;
            sorting_list* temp_list;
            sorting_list* closestto_first;
            sorting_list* closestto_last;

            for(int i = 0; i < 8; ++i) {
                sorting_list current_item = adjacent_array[i];
                
                if(current_item.num_dissimilar != 0) {
                    num_adjacent++;

                    if(num_adjacent == 1) {
                        first = &current_item;
                        temp_list = &current_item;
                    }

                    else {
                        temp_list->next = &current_item;
                        temp_list = &current_item;
                    }
                    current_item.distanceto_first = distance_between(shape->first_boundary->chunk_p, current_item.chunk);
                    current_item.distanceto_last = distance_between(shape->boundaries->chunk_p, current_item.chunk);
                }
            }

            if(first == NULL) {
                LOG_ERR("no adjacents were boundary chunks!");
                setError(ASSUMPTION_WRONG);
                return;
            }

            else if(num_adjacent == 8) {
                LOG_ERR("no dissimilar chunks near current chunk!");
                setError(ASSUMPTION_WRONG);
                return;
            }
        }
    }
}
