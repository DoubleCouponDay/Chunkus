#include "splitting.h"

#include "chunkmap.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <Handleapi.h>
#include <vadefs.h>
#elif __linux__
#include <pthread.h>
#endif

splits* create_splits(int width, int height);
void split_single_chunk(chunkmap* map, split* split_out, int x, int y, int offset_x, int offset_y, float threshold_2);
bool are_two_colours_similar(pixel a, pixel b, float threshold_2);

typedef struct split_data
{
    const chunkmap* map;
    split* split;
    int x_offset;
    int y_offset;
    float threshold_2;
} split_data;

void thread_split(void* split_data);

void split_chunks(chunkmap* map, splits* splits_out, float threshold)
{
    splits_out = create_splits(map->map_width, map->map_height);
    float threshold_2 = threshold * threshold;

    int x_offsets[8] = { -1, 0, +1, +1, +1, 0, -1, -1 };
    int y_offsets[8] = { +1, +1, +1, 0, -1, -1, -1, 0 };
    split_data thread_data[8] = { 0 };
    // pthread_t threads[8] = { 0 };

    for (int i = 0; i < 8; ++i)
    {
        split_data* t_dat = &thread_data[i];
        t_dat->map = map;
        t_dat->split = &splits_out->splits[i];
        t_dat->x_offset = x_offsets[i];
        t_dat->y_offset = y_offsets[i];
        t_dat->threshold_2 = threshold_2;
        // spawn thread 
        // threads[i] = pthread_create(thread_split, t_dat);
    }

    for (int i = 0; i < 8; ++i)
    {
        // join all threads
        // pthread_join(threads[i]);
    }
}

void split_single_chunk(chunkmap* map, split* split_out, int x, int y, int offset_x, int offset_y, float threshold_2)
{
    split_node* node = &split_out->nodes[x][y];
    node->color = map->groups_array_2d[x][y].average_colour;
    int final_x = x + offset_x;
    int final_y = y + offset_y;

    if (final_x < 0 || final_x >= map->map_width ||
        final_y < 0 || final_y >= map->map_height) {
        node->is_boundary = true;
    }

    pixel a = map->groups_array_2d[final_x][final_y].average_colour;
    pixel b = map->groups_array_2d[x][y].average_colour;

    node->is_boundary = are_two_colours_similar(a, b, threshold_2);
}

bool are_two_colours_similar(pixel a, pixel b, float threshold_2)
{
    int r_diff = (int)a.r - (int)b.r;
    int g_diff = (int)a.g - (int)b.g;
    int b_diff = (int)a.b - (int)b.b;
    int mag_2 = r_diff * r_diff + g_diff * g_diff + b_diff * b_diff;
    return (float)mag_2 < threshold_2;
}

splits* create_splits(int width, int height)
{
    splits* out = calloc(1, sizeof(splits));
    for (int i = 0; i < 8; ++i)
    {
        split* thisSplit = &out->splits[i];
        thisSplit->nodes = calloc(width, sizeof(split_node*));
        for (int x = 0; x < width; ++x)
        {
            thisSplit->nodes[x] = calloc(height, sizeof(split_node));
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
        split* s = &splits->splits[i];
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


void thread_split(void* data)
{
    split_data dat = *(split_data*)(data);

    for (int x = 0; x < dat.map->map_width; ++x)
    {
        for (int y = 0; y < dat.map->map_height; ++y)
        {
            split_single_chunk(dat.map, dat.split, x, y, dat.x_offset, dat.y_offset, dat.threshold_2);
        }
    }
}

