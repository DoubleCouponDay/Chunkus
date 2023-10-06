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

splits* create_splits(int width, int height)
{
    splits* out = calloc(1, sizeof(splits));

    for (int i = 0; i < NUM_SPLITS; ++i)
    {
        split* thisSplit = &out->splits[i];
        thisSplit->nodes = calloc(1, width * sizeof(split_node*));
        for (int x = 0; x < width; ++x)
        {
            thisSplit->nodes[x] = calloc(1, height * sizeof(split_node));
        }
    }
    out->splits_width = width;
    return out;
}

void split_single_chunk(chunkmap* map, split* split_out, int x, int y, int offset_x, int offset_y, float threshold)
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

    node->is_boundary = colours_are_similar(a, b, threshold);
}

typedef struct split_data
{
    chunkmap* map;
    split* split;
    int x_offset;
    int y_offset;
    float threshold;
} split_data;

void* thread_split(void* data)
{
    split_data dat = *(split_data*)(data);

    for (int x = 0; x < dat.map->map_width; ++x)
    {
        for (int y = 0; y < dat.map->map_height; ++y)
        {
            split_single_chunk(dat.map, dat.split, x, y, dat.x_offset, dat.y_offset, dat.threshold);
        }
    }
    #ifdef _WIN32
    _endthreadex(NULL);
    #elif __linux__
    pthread_exit(NULL);
    #endif
}

splits* split_chunks(chunkmap* map, float threshold)
{
    splits* splits_out = create_splits(map->map_width, map->map_height);

    int x_offsets[NUM_SPLITS] = { -1, 0, +1, +1, +1, 0, -1, -1 };
    int y_offsets[NUM_SPLITS] = { +1, +1, +1, 0, -1, -1, -1, 0 };
    split_data thread_data[NUM_SPLITS] = { 0 };
    #ifdef _WIN32
    uintptr_t threads[NUM_SPLITS] = { 0 };
    #elif __linux__
    pthread_t* threads[NUM_SPLITS] = { 0 };
    #endif

    for (int i = 0; i < NUM_SPLITS; ++i)
    {
        split_data* t_dat = &thread_data[i];
        t_dat->map = map;
        t_dat->split = &splits_out->splits[i];
        t_dat->x_offset = x_offsets[i];
        t_dat->y_offset = y_offsets[i];
        t_dat->threshold = threshold;

        #if _WIN32
        threads[i] = _beginthreadex(NULL, 0, thread_split, t_dat, 0, NULL);
        #elif __linux__
        threads[i] = (pthread_t*) calloc(1, sizeof(pthread_t));
        pthread_create(threads[i], NULL, thread_split, t_dat);
        #endif
    }

    for (int i = 0; i < 8; ++i)
    {
        // join all threads
        #ifdef _WIN32
        WaitForSingleObjectEx((HANDLE)threads[i], INFINITE, false);
        CloseHandle(threads[i]);
        #elif __linux__
        pthread_join(*threads[i], NULL);
        free((void*)*threads[i]);
        #endif
    }
    return splits_out;
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
        free(s->nodes);
    }
    free(splits);
}
