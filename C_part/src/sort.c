#include "sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "chunkmap.h"
#include "utility/logger.h"
#include "utility/error.h"
#include "utility/vec.h"
#include "prune.h"
#include "utility/defines.h"

enum {
    ADJACENT_COUNT = 9
};

void swap_items(pixelchunk** array, unsigned long a, unsigned long b) {
    pixelchunk* tmp = array[a];
    array[a] = array[b];
    array[b] = tmp;
}

void dont_skip_corners(pixelchunk** array, unsigned long eligiblesubjects[ADJACENT_COUNT], pixelchunk* subject, pixelchunk* previous, 
                        unsigned long eligible_count, unsigned long next, unsigned long length) {

    float smallest_angle = getpi() * 2.f; // set to largest possible radian to begin with
    pixelchunk* most_eligible = NULL;
    unsigned long most_eligible_index = 0;

    for (unsigned long q = 0; q < eligible_count; ++q) {
        pixelchunk* eligible = array[eligiblesubjects[q]];
        float angle = calculate_angle_between(eligible, subject, previous);

        if (angle < smallest_angle)
        {
            smallest_angle = angle;
            most_eligible = eligible;
            most_eligible_index = eligiblesubjects[q];
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

void bubble_sort(pixelchunk** array, unsigned long a, unsigned long length) {
    bool allsorted = false;

    while(allsorted == false) {        
        //unsigned long next = start + 1;

        if(a + 1 >= length) {
            allsorted = true;
            return;
        }
        unsigned long eligiblesubjects[ADJACENT_COUNT] = {0};
        pixelchunk* a_chunk = array[a];
        unsigned long eligible_count = 0;
        pixelchunk* a_prev_chunk = (a ? array[a - 1] : NULL);

        for(unsigned long b = a + 1; b < length; ++b) {
            pixelchunk* b_chunk = array[b];

            if(chunk_is_adjacent(b_chunk, a_chunk)) {
                if(eligible_count == ADJACENT_COUNT) {
                    LOG_ERR("adjacent chunks are larger than known size!");
                    setError(ASSUMPTION_WRONG);
                    return;
                }
                eligiblesubjects[eligible_count] = b;
                ++eligible_count;            
            }
        }
        dont_skip_corners(array, eligiblesubjects, a_chunk, a_prev_chunk, eligible_count, a + 1, length);
        ++a;
    }
}

pixelchunk** convert_boundary_list_toarray(pixelchunk_list* holder, unsigned long length) {
    pixelchunk** output = calloc(1, sizeof(pixelchunk*) * length);
    pixelchunk_list* current = holder;

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

void sort_boundary(chunkmap* map) {
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
        prune_boundary(shape->boundaries);
        shape = shape->next;
        free(array);
    }
}



