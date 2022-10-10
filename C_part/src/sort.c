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

typedef struct sorting_index
{
    short version;
    pixelchunk* chunk;
} sorting_index;

// 2D Canvas of indexes into chunkmap
typedef struct sorting_canvas
{
    int width;
    int height;
    sorting_index* pixels;
} sorting_canvas;

sorting_canvas* create_sorting_canvas(int width, int height)
{
    sorting_canvas* out = calloc(1, sizeof(sorting_canvas));

    int count = width * height;
    if (count < 1)
        return out;

    out->width = width;
    out->height = height;
    out->pixels = calloc(count, sizeof(sorting_index));

    return out;
}

void free_sorting_canvas(sorting_canvas* canvas)
{
    if (!canvas)
        return;

    if (canvas->pixels)
        free(canvas->pixels);
    free(canvas);
}

void set_sorting_canvas(sorting_canvas* canvas, int x, int y, pixelchunk* value, int version)
{
    if (!canvas)
        return;

    if (x < 0 || y < 0 || x >= canvas->width || y >= canvas->height)
    {
        LOG_ERR("Out of bounds position given!");
        return;
    }

    canvas->pixels[y * canvas->width + x].version = version;
    canvas->pixels[y * canvas->width + x].chunk = value;
}

pixelchunk* get_sorting_canvas(sorting_canvas* canvas, int x, int y, int version)
{
    if (!canvas)
        return 0;

    if (x < 0 || y < 0 || x >= canvas->width || y >= canvas->height)
    {
        return 0;
    }

    if (canvas->pixels[y * canvas->width + x].version == version)
        return canvas->pixels[y * canvas->width + x].chunk;
    return 0;
}
enum {
    ADJACENT_COUNT = 8
};

bool chunk_is_adjacent(pixelchunk* current, pixelchunk* subject) {
    int abs_x_diff = abs(subject->location.x - current->location.x);
    int abs_y_diff = abs(subject->location.y - current->location.y);
    bool xisclose = abs_x_diff <= 1;
    bool yisclose = abs_y_diff <= 1;
    return xisclose && yisclose;
}

void swap_items(pixelchunk** array, unsigned long item1, unsigned long item2) {
    pixelchunk* tmp = array[item1];
    array[item1] = array[item2];
    array[item2] = tmp;
}

void dont_skip_corners(pixelchunk** array, unsigned long eligiblesubjects[ADJACENT_COUNT], pixelchunk* subject, pixelchunk* previous, 
                        unsigned long eligible_count, unsigned long next, unsigned long length) {

    float smallest_angle = getpi() * 2.f; // set to largest possible radian to begin with
    pixelchunk* most_eligible = NULL;
    unsigned long most_eligible_index = 0;

    for (unsigned long i = 0; i < eligible_count; ++i) {
        pixelchunk* eligible = array[eligiblesubjects[i]];
        float angle = calculate_angle_between(eligible, subject, previous);

        if (angle < smallest_angle)
        {
            smallest_angle = angle;
            most_eligible = eligible;
            most_eligible_index = eligiblesubjects[i];
        }
    }

    if (most_eligible)
    {
        swap_items(array, most_eligible_index, next);
    }

    else
    {
        return;
    }
}

void bubble_sort(pixelchunk** array, unsigned long start, unsigned long length) {
    bool allsorted = false;

    while(allsorted == false) {        
        unsigned long next = start + 1;

        if(next >= length) {
            allsorted = true;
            return;
        }
        unsigned long eligiblesubjects[ADJACENT_COUNT] = {0};
        pixelchunk* starting_chunk = array[start];
        unsigned long eligible_count = 0;
        pixelchunk* previous = (start ? array[start - 1] : NULL);

        for(unsigned long i = start + 1; i < length; ++i) {
            pixelchunk* current_chunk = array[i];

            if(current_chunk == starting_chunk) {
                continue; //sadly this happens a lot
            }

            if(chunk_is_adjacent(current_chunk, starting_chunk)) {
                if(eligible_count > ADJACENT_COUNT) {
                    LOG_ERR("adjacent chunks are more numerous than possible adjacent chunks!");
                    setError(ASSUMPTION_WRONG);
                    return;
                }
                eligiblesubjects[eligible_count] = i;
                ++eligible_count;            
            }
        }
        dont_skip_corners(array, eligiblesubjects, starting_chunk, previous, eligible_count, next, length);
        ++start;
    }
}

pixelchunk** convert_boundary_list_toarray(pixelchunk_list* list, unsigned long length) {
    pixelchunk** output = calloc(1, sizeof(pixelchunk*) * length);
    pixelchunk_list* current = list;

    for(unsigned long i = 0; i < length; ++i) {
        output[i] = current->chunk_p;
        current = current->next;
    }
    return output;
}


void convert_array_to_boundary_list(pixelchunk** array, pixelchunk_list* output, unsigned long length) {
    pixelchunk_list* current = output;

    for(unsigned long i = 0; i < length; ++i) {
        current->chunk_p = array[i];
        current = current->next;
    }
}

typedef struct pixelchunk_vector
{
    pixelchunk** first; // Points to the first element
    pixelchunk** last; // Points to the space after the last element (ie. the end of the array)
    pixelchunk** realend; // Points to the end of the allocated memory (it points to out of bounds memory)
} pc_vector;

pc_vector pc_create(unsigned long size, unsigned long capacity) 
{
    if (capacity < size) 
    {
        capacity = size;
    }
    pc_vector vector;
    vector.first = calloc(1, sizeof(pixelchunk*) * capacity);
    vector.last = vector.first + size;
    vector.realend = vector.first + capacity;
    return vector;
}

int pc_vector_size(pc_vector* vector)
{
    return vector->last - vector->first;
}

int pc_vector_capacity(pc_vector* vector)
{
    return vector->realend - vector->first;
}

void pc_append(pc_vector* vector, pixelchunk* chunk)
{
    if (vector->first == NULL)
    {
        vector->first = calloc(1, sizeof(pixelchunk*));
        vector->last = vector->first + 1;
        vector->realend = vector->first + 1;
    }
    else if (vector->last >= vector->realend)
    {
        unsigned long size = pc_vector_size(vector);
        vector->first = realloc(vector->first, sizeof(pixelchunk*) * (size + 5));
        vector->last = vector->first + size;
        vector->realend = vector->first + (size + 5);
    }

    *vector->last = chunk;
    ++vector->last;
}

pixelchunk* pc_at(pc_vector* vector, int i)
{
    return vector->first[i];
}

// Returns previous value
pixelchunk* pc_set(pc_vector* vector, unsigned long i, pixelchunk* chunk)
{
    pixelchunk* old = vector->first[i];
    vector->first[i] = chunk;
    return old;
}

void pc_insert(pc_vector* vector, unsigned long position, pixelchunk* val)
{
    if (position < 0 || position > pc_vector_size(vector))
    {
        return;
    }
    if (vector->first == NULL)
    {
        vector->first = calloc(1, sizeof(pixelchunk*));
        vector->last = vector->first + 1;
        vector->realend = vector->first + 1;
        *vector->first = val;
        return;
    }
    else if (vector->last + 1 > vector->realend)
    {
        vector->first = realloc(vector->first, sizeof(pixelchunk*) * ((vector->last - vector->first) + 10));
        vector->realend += 10;
    }

    for (pixelchunk** iter = vector->last - 1; iter >= vector->first + position; --iter)
    {
        pixelchunk** next = iter + 1;
        *next = *iter;
    }

    vector->first[position] = val;
}

void pc_erase(pc_vector* vector, unsigned long position)
{
    if (position < 0 || position >= pc_vector_size(vector))
    {
        return;
    }

    for (pixelchunk** iter = vector->first + position; iter < vector->last - 1; ++iter)
    {
        pixelchunk** next = iter + 1;
        *iter = *next;
    }

    --vector->last;
}

void pc_free(pc_vector* vector)
{
    free(vector->first);
    vector->first = NULL;
    vector->last = NULL;
    vector->realend = NULL;
}

enum AdjacentPosition
{
    TOP_LEFT = 0,
    TOP_MIDDLE = 1,
    TOP_RIGHT = 2,
    MIDDLE_LEFT = 3,
    MIDDLE_RIGHT = 4,
    BOTTOM_LEFT = 5,
    BOTTOM_MIDDLE = 6,
    BOTTOM_RIGHT = 7
};

void find_adjacents(pixelchunk* chunk, pixelchunk* adjacents[ADJACENT_COUNT], sorting_canvas* canvas, int canvas_version)
{
    adjacents[TOP_LEFT]        = get_sorting_canvas(canvas, chunk->location.x - 1, chunk->location.y - 1, canvas_version);
    adjacents[TOP_MIDDLE]      = get_sorting_canvas(canvas, chunk->location.x + 0, chunk->location.y - 1, canvas_version);
    adjacents[TOP_RIGHT]       = get_sorting_canvas(canvas, chunk->location.x + 1, chunk->location.y - 1, canvas_version);
    adjacents[MIDDLE_LEFT]     = get_sorting_canvas(canvas, chunk->location.x - 1, chunk->location.y + 0, canvas_version);
    adjacents[MIDDLE_RIGHT]    = get_sorting_canvas(canvas, chunk->location.x + 1, chunk->location.y + 0, canvas_version);
    adjacents[BOTTOM_LEFT]     = get_sorting_canvas(canvas, chunk->location.x - 1, chunk->location.y + 1, canvas_version);
    adjacents[BOTTOM_MIDDLE]   = get_sorting_canvas(canvas, chunk->location.x + 0, chunk->location.y + 1, canvas_version);
    adjacents[BOTTOM_RIGHT]    = get_sorting_canvas(canvas, chunk->location.x + 1, chunk->location.y + 1, canvas_version);
}

float calculate_ccw_angle(vector2 a, vector2 b)
{
    float dot = a.x * b.x + a.y * b.y;
    float det = a.x * b.y - a.y * b.x;
    float angle = atan2(det, dot);
    if (angle < 0)
    {
        angle += 2 * getpi();
    }
    return angle;
}

float calculate_cw_angle(vector2 a, vector2 b)
{
    return calculate_ccw_angle(b, a);
}

void iterate_shape_boundaries(chunkshape* shape, sorting_canvas* canvas, int version)
{
    pc_vector vector = pc_create(0, shape->boundaries_length);
    
    // Go through boundaries and append the next boundary chunk (duplicates if necessary) to the vector
    int count = 0;
    int max_count = shape->boundaries_length * 2;
    pixelchunk* current = shape->boundaries->chunk_p;

    pc_append(&vector, current);

    pixelchunk* prev = current;

    pixelchunk* adjacents[ADJACENT_COUNT];

    memset(adjacents, 0, sizeof(adjacents));
    find_adjacents(current, adjacents, canvas, version);

    if (adjacents[TOP_LEFT])
        current = adjacents[TOP_LEFT];
    else if (adjacents[TOP_MIDDLE])
        current = adjacents[TOP_MIDDLE];
    else if (adjacents[TOP_RIGHT])
        current = adjacents[TOP_RIGHT];
    else if (adjacents[MIDDLE_RIGHT])
        current = adjacents[MIDDLE_RIGHT];
    else if (adjacents[BOTTOM_RIGHT])
        current = adjacents[BOTTOM_RIGHT];
    else if (adjacents[BOTTOM_MIDDLE])
        current = adjacents[BOTTOM_MIDDLE];
    else if (adjacents[BOTTOM_LEFT])
        current = adjacents[BOTTOM_LEFT];


    while (current != pc_at(&vector, 0) && count < max_count)
    {
        pc_append(&vector, current);

        memset(adjacents, 0, sizeof(adjacents));
        find_adjacents(current, adjacents, canvas, version);

        float smallest_angle = getpi() * 2.0f;
        pixelchunk* smallest_angle_chunk = NULL;
        vector2 dir = { prev->location.x - current->location.x, prev->location.y - current->location.y };
        
        for (int i = 0; i < ADJACENT_COUNT; ++i)
        {
            pixelchunk* adjacent = adjacents[i];

            if (!adjacent || adjacent == prev)
                continue;

            vector2 next_dir = {adjacent->location.x - current->location.x, adjacent->location.y - current->location.y};
            float angle = calculate_ccw_angle(dir, next_dir);

            if (angle < smallest_angle)
            {
                smallest_angle = angle;
                smallest_angle_chunk = adjacent;
            }
        }

        if (!smallest_angle_chunk)
        {
            // Try going backward instead
            smallest_angle_chunk = prev;
        }

        prev = current;
        current = smallest_angle_chunk;
        ++count;
    }

    // Copy the vector back to the list
    free_pixelchunklist(shape->boundaries);
    if (!pc_vector_size(&vector))
    {
        return;
    }
    pixelchunk_list* first = (pixelchunk_list*)malloc(sizeof(pixelchunk_list));
    first->chunk_p = pc_at(&vector, 0);
    first->next = NULL;
    first->first = first;
    shape->boundaries = first;
    for (int i = 1; i < pc_vector_size(&vector); ++i)
    {
        pixelchunk* chunk = pc_at(&vector, i);
        pixelchunk_list* list = (pixelchunk_list*)malloc(sizeof(pixelchunk_list));
        list->chunk_p = chunk;
        list->next = NULL;
        list->first = first;
        shape->boundaries->next = list;
        shape->boundaries = list;
    }
    shape->boundaries = first;
    shape->boundaries_length = pc_vector_size(&vector);
    pc_free(&vector);
}

void sort_boundary(chunkmap* map) {

    sorting_canvas* canvas = create_sorting_canvas(map->map_width, map->map_height);
    int version = 0;

    chunkshape* shape = map->shape_list;

    while (shape)
    {
        pixelchunk** array = convert_boundary_list_toarray(shape->boundaries, shape->boundaries_length);
        bubble_sort(array, 0, shape->boundaries_length);

        if(isBadError()) {
            LOG_ERR("bubble_sort failed with code: %d", getLastError());
            return;
        }
        convert_array_to_boundary_list(array, shape->boundaries, shape->boundaries_length);
        pixelchunk_list* last = shape->boundaries;

        while(last && last->next) {
            last = last->next;
        }

        if (!chunk_is_adjacent(last->chunk_p, shape->boundaries->chunk_p))
        {
            ++version;
            for (pixelchunk_list* list = shape->boundaries; list != 0; list = list->next)
            {
                set_sorting_canvas(canvas, list->chunk_p->location.x, list->chunk_p->location.y, list->chunk_p, version);
            }

            iterate_shape_boundaries(shape, canvas, version);
        }

        shape = shape->next;
        free(array);
    }

    free_sorting_canvas(canvas);
}
