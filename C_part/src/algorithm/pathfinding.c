#include "pathfinding.h"

#include <stdbool.h>

#include "sort.h"
#include "../chunkmap.h"

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
