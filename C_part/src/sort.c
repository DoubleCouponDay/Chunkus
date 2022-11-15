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
    pixelchunk* dissimilar_chunk;
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
    sorting_item output = {subject, NULL, 0, false};

    if(subject == NULL)
        return output;

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

    if(topright_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = top_right;
    }

    bool top_dissimilar = top != NULL && colours_are_similar(subject->average_colour, top->average_colour, quadrant->options->threshold) == false;
    
    if(top_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = top;
    }

    bool topleft_dissimilar = top_left != NULL && colours_are_similar(subject->average_colour, top_left->average_colour, quadrant->options->threshold) == false;

    if(topleft_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = top_left;
    }

    bool left_dissimilar = left != NULL && colours_are_similar(subject->average_colour, left->average_colour, quadrant->options->threshold) == false;
    
    if(left_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = left;
    }

    bool botleft_dissimilar = bot_left != NULL && colours_are_similar(subject->average_colour, bot_left->average_colour, quadrant->options->threshold) == false;
    
    if(botleft_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = bot_left;
    }

    bool bot_dissimilar = bot != NULL && colours_are_similar(subject->average_colour, bot->average_colour, quadrant->options->threshold) == false;
    
    if(bot_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = bot;
    }

    bool botright_dissimilar = bot_right != NULL && colours_are_similar(subject->average_colour, bot_right->average_colour, quadrant->options->threshold) == false;
    
    if(botright_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = bot_right;
    }

    bool right_dissimilar = right != NULL && colours_are_similar(subject->average_colour, right->average_colour, quadrant->options->threshold) == false;

    if(right_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = right;
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

void not_adjacent_firstlast(Quadrant* quadrant, chunkshape* shape) {
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

        //if adjacent is in the quadrant and similar to a known boundary chunk (current) and adjacent to at least one dissimilar chunk and not already in a boundary
        sorting_item topright_item = is_boundary_chunk(quadrant, top_right);
        topright_item.is_boundary = top_right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top_right->average_colour, quadrant->options->threshold) && topright_item.dissimilar_chunk != NULL && top_right->boundary_chunk_in == NULL;
        sorting_item top_item = is_boundary_chunk(quadrant, top);
        top_item.is_boundary = top != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top->average_colour, quadrant->options->threshold) && top_item.dissimilar_chunk != NULL && top->boundary_chunk_in == NULL;
        sorting_item topleft_item = is_boundary_chunk(quadrant, top_left);
        topleft_item.is_boundary = top_left != NULL && top_left->boundary_chunk_in == NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top_left->average_colour, quadrant->options->threshold) && topleft_item.dissimilar_chunk != NULL;
        sorting_item left_item = is_boundary_chunk(quadrant, left);
        left_item.is_boundary = left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, left->average_colour, quadrant->options->threshold) && left_item.dissimilar_chunk != NULL && left->boundary_chunk_in == NULL;
        sorting_item botleft_item = is_boundary_chunk(quadrant, bot_left);
        botleft_item.is_boundary = bot_left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold) && botleft_item.dissimilar_chunk != NULL && bot_left->boundary_chunk_in == NULL;
        sorting_item bot_item = is_boundary_chunk(quadrant, bot);
        bot_item.is_boundary = bot != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot->average_colour, quadrant->options->threshold) && bot_item.dissimilar_chunk != NULL && bot->boundary_chunk_in == NULL;
        sorting_item botright_item = is_boundary_chunk(quadrant, bot_right);
        botright_item.is_boundary = bot_right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_right->average_colour, quadrant->options->threshold) && botright_item.dissimilar_chunk != NULL && bot_right->boundary_chunk_in == NULL;
        sorting_item right_item = is_boundary_chunk(quadrant, right);
        right_item.is_boundary = right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, right->average_colour, quadrant->options->threshold) && right_item.dissimilar_chunk != NULL && right->boundary_chunk_in == NULL;

        sorting_item adjacent_array[] = {
            topright_item,
            top_item,
            topleft_item,
            left_item,
            botleft_item,
            bot_item,
            botright_item,
            right_item
        };

        int num_have_adjacent = 0;
        sorting_item highest;
        int highest_adjacent = 0;

        //find the adjacent boundary chunk closest to the most dissimilar chunks
        for(int i = 0; i < 8; ++i) {
            sorting_item current_item = adjacent_array[i];
            
            if(current_item.is_boundary) {
                ++num_have_adjacent;
                
                if(current_item.num_dissimilar > highest_adjacent) {
                    highest_adjacent = current_item.num_dissimilar;
                    highest = adjacent_array[i];
                }
            }
        }

        if(num_have_adjacent == 0) {
            sort_focus->next = NULL;
            sort_focus = NULL;
            return;
        }

        else if(num_have_adjacent == 8) {
            LOG_ERR("no dissimilar chunks!");
            setError(ASSUMPTION_WRONG);
            return;
        }

        else if(highest.chunk == NULL) {
            LOG_ERR("highest not found");
            setError(ASSUMPTION_WRONG);
            return;
        }
        pixelchunk_list* list = create_boundaryitem(highest.chunk);
        shape->boundaries->next = list;
        shape->boundaries = list;
        sort_focus = list;
        zip_seam(quadrant, highest.chunk, highest.dissimilar_chunk);
    }
}

/// @brief returns whether the current chunk was added to the boundary or not.
/// Assume current->shape_chunk_in is NULL
/// @param quadrant 
/// @param shape 
/// @param current 
bool sort_boundary_chunk(Quadrant* quadrant, chunkshape* shape, pixelchunk* current) {
    bool current_sorted = false;

    if(shape->boundaries == NULL) { //dont sort if first chunk
        current_sorted = true;
        pixelchunk_list* list = create_boundaryitem(current);
        list->next = NULL;
        shape->boundaries = list;
    }

    else if(is_adjacent(current, shape->boundaries)) { //chunk is adjacent to last and is not first
        pixelchunk_list* list = create_boundaryitem(current);
        shape->boundaries->next = list; //also accounts for boundary flipping over at the second boundary item
        shape->boundaries = list;
        current_sorted = true;
    }

    else if(is_adjacent(current, shape->first_boundary)) { //chunk is adjacent to first and is not last
        pixelchunk_list* list = create_boundaryitem(current);
        list->next = shape->first_boundary;
        shape->first_boundary = list;
        current_sorted = true;
    }

    else { //shapes boundary on next scanline starts back to front away from first or last boundary chunk
        not_adjacent_firstlast(quadrant, shape);
    }
    return current_sorted;
}
