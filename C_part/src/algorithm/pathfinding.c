#include "pathfinding.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "../chunkmap.h"

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

pixelchunk* get_at(Layer* layer, int x, int y)
{
    if (x < 0 || x >= layer->map->map_width ||
        y < 0 || y >= layer->map->map_height)
    {
        return NULL;
    }
    return &layer->map->groups_array_2d[x][y];
}

/// @brief returns the info about the first dissimilar chunk
/// @param layer 
/// @param subject 
/// @return 
sorting_item is_boundary_chunk(Layer* layer, pixelchunk* subject) {
    sorting_item output = {subject, NULL, 0, false};

    if(subject == NULL)
        return output;

    int current_x = subject->location.x;
    int current_y = subject->location.y;
    pixelchunk* top_right = get_at(layer, current_x + 1, current_y - 1);
    pixelchunk* top = get_at(layer, current_x, current_y - 1);
    pixelchunk* top_left = get_at(layer, current_x - 1, current_y - 1);
    pixelchunk* left = get_at(layer, current_x - 1, current_y);
    pixelchunk* bot_left = get_at(layer, current_x - 1, current_y + 1);
    pixelchunk* bot = get_at(layer, current_x, current_y + 1);
    pixelchunk* bot_right = get_at(layer, current_x + 1, current_y + 1);
    pixelchunk* right = get_at(layer, current_x + 1, current_y);

    bool topright_dissimilar = top_right != NULL && colours_are_similar(subject->average_colour, top_right->average_colour, layer->options->threshold) == false;

    if(topright_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = top_right;
    }

    bool top_dissimilar = top != NULL && colours_are_similar(subject->average_colour, top->average_colour, layer->options->threshold) == false;
    
    if(top_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = top;
    }

    bool topleft_dissimilar = top_left != NULL && colours_are_similar(subject->average_colour, top_left->average_colour, layer->options->threshold) == false;

    if(topleft_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = top_left;
    }

    bool left_dissimilar = left != NULL && colours_are_similar(subject->average_colour, left->average_colour, layer->options->threshold) == false;
    
    if(left_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = left;
    }

    bool botleft_dissimilar = bot_left != NULL && colours_are_similar(subject->average_colour, bot_left->average_colour, layer->options->threshold) == false;
    
    if(botleft_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = bot_left;
    }

    bool bot_dissimilar = bot != NULL && colours_are_similar(subject->average_colour, bot->average_colour, layer->options->threshold) == false;
    
    if(bot_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = bot;
    }

    bool botright_dissimilar = bot_right != NULL && colours_are_similar(subject->average_colour, bot_right->average_colour, layer->options->threshold) == false;
    
    if(botright_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = bot_right;
    }

    bool right_dissimilar = right != NULL && colours_are_similar(subject->average_colour, right->average_colour, layer->options->threshold) == false;

    if(right_dissimilar) {
        ++output.num_dissimilar;
        output.dissimilar_chunk = right;
    }
    return output;
}

bool pathfind_shape(Layer* layer, chunkshape* shape, pixelchunk* current) {
    bool used_current = false;
    pixelchunk_list* sort_focus = shape->boundaries;

    while(sort_focus != NULL) {
        int current_x = sort_focus->chunk_p->location.x;
        int current_y = sort_focus->chunk_p->location.y;
        int last_x = shape->boundaries->chunk_p->location.x;
        int last_y = shape->boundaries->chunk_p->location.y;

        pixelchunk* top_right = get_at(layer, current_x + 1, current_y - 1);
        pixelchunk* top = get_at(layer, current_x, current_y - 1);
        pixelchunk* top_left = get_at(layer, current_x - 1, current_y - 1);
        pixelchunk* left = get_at(layer, current_x - 1, current_y);
        pixelchunk* bot_left = get_at(layer, current_x - 1, current_y + 1);
        pixelchunk* bot = get_at(layer, current_x, current_y + 1);
        pixelchunk* bot_right = get_at(layer, current_x + 1, current_y + 1);
        pixelchunk* right = get_at(layer, current_x + 1, current_y);

        //if adjacent is similar to a known boundary chunk (current) and adjacent to at least one dissimilar chunk and not already in a boundary
        sorting_item topright_item = is_boundary_chunk(layer, top_right);
        topright_item.is_boundary = top_right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top_right->average_colour, layer->options->threshold) && topright_item.dissimilar_chunk != NULL && top_right->boundary_chunk_in == NULL;
        sorting_item top_item = is_boundary_chunk(layer, top);
        top_item.is_boundary = top != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top->average_colour, layer->options->threshold) && top_item.dissimilar_chunk != NULL && top->boundary_chunk_in == NULL;
        sorting_item topleft_item = is_boundary_chunk(layer, top_left);
        topleft_item.is_boundary = top_left != NULL && top_left->boundary_chunk_in == NULL && colours_are_similar(sort_focus->chunk_p->average_colour, top_left->average_colour, layer->options->threshold) && topleft_item.dissimilar_chunk != NULL;
        sorting_item left_item = is_boundary_chunk(layer, left);
        left_item.is_boundary = left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, left->average_colour, layer->options->threshold) && left_item.dissimilar_chunk != NULL && left->boundary_chunk_in == NULL;
        sorting_item botleft_item = is_boundary_chunk(layer, bot_left);
        botleft_item.is_boundary = bot_left != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_left->average_colour, layer->options->threshold) && botleft_item.dissimilar_chunk != NULL && bot_left->boundary_chunk_in == NULL;
        sorting_item bot_item = is_boundary_chunk(layer, bot);
        bot_item.is_boundary = bot != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot->average_colour, layer->options->threshold) && bot_item.dissimilar_chunk != NULL && bot->boundary_chunk_in == NULL;
        sorting_item botright_item = is_boundary_chunk(layer, bot_right);
        botright_item.is_boundary = bot_right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, bot_right->average_colour, layer->options->threshold) && botright_item.dissimilar_chunk != NULL && bot_right->boundary_chunk_in == NULL;
        sorting_item right_item = is_boundary_chunk(layer, right);
        right_item.is_boundary = right != NULL && colours_are_similar(sort_focus->chunk_p->average_colour, right->average_colour, layer->options->threshold) && right_item.dissimilar_chunk != NULL && right->boundary_chunk_in == NULL;

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
            return used_current;
        }

        else if(num_have_adjacent == 8) {
            LOG_ERR("no dissimilar chunks!");
            setError(ASSUMPTION_WRONG);
            return used_current;
        }

        else if(highest.chunk == NULL) {
            LOG_ERR("highest not found");
            setError(ASSUMPTION_WRONG);
            return used_current;
        }

        if(highest.chunk == current)
            used_current = true;

        pixelchunk_list* list = create_boundaryitem(highest.chunk);
        shape->boundaries->next = list;
        shape->boundaries = list;
        sort_focus = list;
        zip_seam(highest.chunk, highest.dissimilar_chunk);
    }
    shape->path_closed = true;
    return used_current;
}

void pathfind_shapes(Layer* layer, chunkmap* map, bool** aggregation)
{
    for (int x = 0; x < map->map_width; ++x)
    {
        for (int y = 0; y < map->map_height; ++y)
        {
            pixelchunk* chunk = &map->groups_array_2d[x][y];
            bool is_boundary = aggregation[x][y];

            if (chunk->boundary_chunk_in != NULL || is_boundary == false)
                continue;

            chunkshape* shape = generate_chunkshape(map);
            pathfind_shape(layer, shape, chunk);
        }
    }
}
