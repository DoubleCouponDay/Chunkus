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

pixelchunk* get_at(Quadrant* quad, int x, int y);
pixelchunk* get_next(Quadrant* quadrant, coordinate cur_pos, pixelchunk* prev);
pixelchunk_list* append_list(pixelchunk_list* to, pixelchunk* of);

void sort_boundary(Quadrant* quadrant) {
    //iterate through all shapes
    //start with first chunk in boundary
    //if chunk is adjacent and shape_chunk_in is equal and boundary_chunk_in is equal and chunk is not previous boundary chunk: use it
    //set current chunk equal to the next boundary chunk and repeat until done?
    //how do we define done?
    chunkmap* map = quadrant->map;
    
    for (chunkshape* shape = map->shape_list; shape; shape = shape->next)
    {
        pixelchunk_list* prev = NULL;
        pixelchunk_list* cur = calloc(1, sizeof(pixelchunk_list));
        cur->chunk_p = shape->boundaries->first->chunk_p;
        cur->first = cur;
        cur->next = NULL;
        coordinate cur_pos = cur->chunk_p->location;

        pixelchunk* next_chunk = get_next(quadrant, cur_pos, NULL);
        if (!next_chunk)
            continue;
        prev = cur;
        cur = append_list(cur, next_chunk);
        
        while(cur && cur != shape->boundaries->first) {
            cur_pos = cur->chunk_p->location;
            next_chunk = get_next(quadrant, cur_pos, prev->chunk_p);
            if (!next_chunk)
                break;
            prev = cur;
            cur = append_list(cur, next_chunk);
        }
    }
}

pixelchunk_list* append_list(pixelchunk_list* to, pixelchunk* of)
{
    pixelchunk_list* next = calloc(1, sizeof(pixelchunk_list));
    next->chunk_p = of;
    next->next = NULL;
    next->first = to->first;

    to->next = next;
    return next;
}

pixelchunk* get_at(Quadrant* quad, int x, int y)
{
    if (x < quad->bounds.startingX || x >= quad->bounds.endingX ||
        y < quad->bounds.startingY || y >= quad->bounds.endingY)
    {
        return NULL;
    }

    return quad->map->groups_array_2d[x][y].boundary_chunk_in;
}

pixelchunk* get_next(Quadrant* quadrant, coordinate cur_pos, pixelchunk* prev)
{
    static coordinate next_offsets[8] = {
        (coordinate){ -1, -1 },
        (coordinate){ +0, -1 },
        (coordinate){ +1, -1 },
        (coordinate){ -1, +0 },
        (coordinate){ +1, +0 },
        (coordinate){ -1, +1 },
        (coordinate){ +0, +1 },
        (coordinate){ +1, +1 }
    };

    for (int next_i = 0; next_i < 8; ++next_i)
    {
        int neighbour_x = cur_pos.x + next_offsets[next_i].x;
        int neighbour_y = cur_pos.y + next_offsets[next_i].y;
        pixelchunk_list* neighbour = get_at(quadrant, neighbour_x, neighbour_y);
        if (neighbour && neighbour != prev)
        {
            return neighbour;
        }
    }
    return NULL;
}
