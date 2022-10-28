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

        //XXXX XXXX
        if(topright_boundary && top_boundary && topleft_boundary && left_boundary && botleft_boundary && bot_boundary && botright_boundary && right_boundary) {
            LOG_ERR("some chunks should be in the other shape but are not.");
            setError(ASSUMPTION_WRONG);
        }

        //XXXX XXXO

        //XXXX XXOX

        //
    }
}
