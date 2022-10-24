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

void free_listitem_if_exists(pixelchunk* chunk) {
    if(chunk->boundary_chunk_in == NULL) {
        return;
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
        pixelchunk* bot_left = get_at(quadrant, current_x - 1, current_y + 1);
        pixelchunk* bot = get_at(quadrant, current_x, current_y + 1);
        pixelchunk* bot_right = get_at(quadrant, current_x + 1, current_y + 1);            
        pixelchunk* right = get_at(quadrant, current_x + 1, current_y);

        bool botleft_similar = colours_are_similar(current->chunk_p->average_colour, bot_left->average_colour, quadrant->options->threshold);

        bool bot_similar = colours_are_similar(current->chunk_p->average_colour, bot->average_colour, quadrant->options->threshold);

        bool botright_similar = colours_are_similar(current->chunk_p->average_colour, bot_right->average_colour, quadrant->options->threshold);
        
        bool right_similar = colours_are_similar(current->chunk_p->average_colour, right->average_colour, quadrant->options->threshold);

        //XXXX
        if(botleft_similar && bot_similar && botright_similar && right_similar) {
            LOG_ERR("some chunks should be in the other shape but are not.");
            setError(ASSUMPTION_WRONG);
        }

        //XXXO
        else if(botleft_similar && bot_similar && botright_similar && !right_similar) {
            bot_left->shape_chunk_in = shape;
            bot_left->boundary_chunk_in = calloc(1, sizeof(pixelchunk_list));
            
        }
        
        //XXOX
        else if(botleft_similar && bot_similar && !botright_similar && right_similar) {

        }
        
        //XXOO
        else if(botleft_similar && bot_similar && !botright_similar && !right_similar) {

        }

        //XOXX
        else if(botleft_similar && !bot_similar && botright_similar && right_similar) {
            
        }

        //XOXO
        else if(botleft_similar && !bot_similar && botright_similar && !right_similar) {

        }

        //XOOX
        else if(botleft_similar && !bot_similar && !botright_similar && right_similar) {

        }

        //XOOO
        else if(botleft_similar && !bot_similar && !botright_similar && !right_similar) {
            LOG_ERR("only botleft is similar!");
            setError(ASSUMPTION_WRONG);
        }

        //OXXX
        else if(!botleft_similar && bot_similar && botright_similar && right_similar) {

        }

        //OXXO
        else if(!botleft_similar && bot_similar && botright_similar && !right_similar) {

        }

        //OXOX
        else if(!botleft_similar && bot_similar && !botright_similar && right_similar) {

        }

        //OXOO
        else if(!botleft_similar && bot_similar && !botright_similar && !right_similar) {
            LOG_ERR("only bot is similar!");
            setError(ASSUMPTION_WRONG);
        }

        //OOXX
        else if(!botleft_similar && !bot_similar && botright_similar && right_similar) {

        }

        //OOXO
        else if(!botleft_similar && !bot_similar && botright_similar && !right_similar) {
            LOG_ERR("only botright is similar!");
            setError(ASSUMPTION_WRONG);
        }

        //OOOX
        else if(!botleft_similar && !bot_similar && !botright_similar && right_similar) {
            LOG_ERR("only right is similar");
            setError(ASSUMPTION_WRONG);
        }

        //OOOO
        else {
            LOG_ERR("none are similar!");
            setError(ASSUMPTION_WRONG);
        }
    }
}
