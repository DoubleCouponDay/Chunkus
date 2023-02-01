#include "splitting.h"

#include "chunkmap.h"

#include <stdlib.h>

splits* create_splits(int width, int height);

void split_chunks(chunkmap* map, splits* splits_out)
{
    splits_out = create_splits(map->map_width);

    int x_offsets[8] = { -1, 0, +1, +1, +1, 0, -1, -1 };
    int y_offsets[8] = { +1, +1, +1, 0, -1, -1, -1, 0 };

    for (int x = 0; x < map->map_width; ++x)
    {
        for (int y = 0; y < map->map_height; ++y)
        {
            for (int i = 0; i < 8; ++i)
            {
                split_node* node = &splits_out->splits[i]->nodes[x][y];
                node->color = map->groups_array_2d[x][y].color;
                int final_x = x + x_offsets[i];
                int final_y = y + y_offsets[i];

                if (final_x < 0 || final_x >= map->map_width ||
                    final_y < 0 || final_y >= map->map_height) {
                    node->is_boundary = true;
                }
            }
        }
    }
}

splits* create_splits(int width, int height)
{
    splits* out = calloc(1, sizeof(splits));
    for (int i = 0; i < 8; ++i)
    {
        split* thisSplit = &(*out)[i];
        thisSplit->nodes = calloc(width, sizeof(split_node*));
        for (int x = 0; x < width; ++x)
        {
            thisSplit.nodes[x] = calloc(height, sizeof(split_node));
        }
    }
    out->splits_width = width;
    return out;
}

void free_splits(splits* splits)
{
    if (!splits)
        return;
    
    if (splits->splits_width <= 0) {
        free(splits);
        return;
    }

    for (int i = 0; i < 8; ++i) {
        split* s = &(*splits)[i];
        if (!s->nodes)
            continue;
        for (int x = 0; x < splits->splits_width; ++x) {
            if (!s->nodes[x])
                continue;
            free(s->nodes[x]);
        }
    }

    free(splits);
}

