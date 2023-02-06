#include "pathfinding.h"

#include "sort.h"


bool chunk_is_border(pixelchunk* chunk, pathfind_input border_lookup);
chunkshape* create_chunkshape(chunkmap* target_map);

void pathfind_shapes(Layer* layer, chunkmap* map, pathfind_input border_lookup)
{
    for (int x = 0; x < map->map_width; ++x)
    {
        for (int y = 0; y < map->map_height; ++y)
        {
            pixelchunk* chunk = &map->groups_array_2d[x][y];

            // Skip chunks already in shapes
            if (chunk->boundary_chunk_in)
                continue;
            
            // Skip chunks that aren't borders/boundaries
            if (!chunk_is_border(chunk, border_lookup))
                continue;

            chunkshape* shape = create_chunkshape(map);
            pathfind_shape(layer, shape, chunk);
        }
    }
}

bool chunk_is_border(pixelchunk* chunk, pathfind_input border_lookup)
{
    // TODO: .... not sure where 'is_boundary' properties are stored yet
    // Perhaps:
    // return chunk->is_boundary;
    // or:
    // return border_lookup[chunk->location.x][chunk->location.y].is_boundary;
    return false; 
}

chunkshape* create_chunkshape(chunkmap* target_map)
{
    chunkshape* new_shape = calloc(1, sizeof(chunkshape));

    new_shape->boundaries = 0;
    new_shape->boundaries_length = 0;
    new_shape->first_boundary = 0;
    new_shape->next = 0;
    new_shape->path_closed = false;

    if (!target_map->first_shape)
        target_map->first_shape = new_shape;
    if (target_map->shape_list)
        target_map->shape_list->next = new_shape;
    target_map->shape_list = new_shape;
    return new_shape;
}

