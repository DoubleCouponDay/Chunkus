#include "sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "utility/defines.h"
#include <math.h>

#include "chunkmap.h"
#include "../test/debug.h"
#include "utility/error.h"
#include "utility/vec.h"
#include "prune.h"

enum {
    ADJACENT_COUNT = 9
};

vector2 calculate_2d_diff(pixelchunk* subject, pixelchunk* previous) {
    int x_diff = subject->location.x - previous->location.x;
    int y_diff = subject->location.y - previous->location.y;
    vector2 diff = { x_diff, y_diff };
    return diff;
}

float calculate_angle_between(pixelchunk* eligible, pixelchunk* subject, pixelchunk* previous) {
    int eligible_x_diff = eligible->location.x - subject->location.x;
    int eligible_y_diff = eligible->location.y - subject->location.y;
    vector2 subject_to_eligible = { eligible_x_diff, eligible_y_diff };
    vector2 diff = calculate_2d_diff(subject, previous);
    float angle = vec_angle_between(diff, subject_to_eligible);
    return angle;
}

void sort_item(pixelchunk** array, pixelchunk* current, unsigned long i, unsigned long next, unsigned long length) {
    pixelchunk* inbetween = array[next];
    array[next] = current;
    array[i] = inbetween;
    //bubble_sort(array, next, length);
}

void dont_skip_corners(pixelchunk** array, unsigned long eligiblesubjects[ADJACENT_COUNT], pixelchunk* subject, pixelchunk* previous, 
                        unsigned long eligible_count, unsigned long next, unsigned long length) {

    float smallest_angle = M_PI * 2.f; // set to largest possible radian to begin with
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
        sort_item(array, most_eligible, most_eligible_index, next, length);
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
        pixelchunk* subject = array[start];
        unsigned long eligible_count = 0;
        pixelchunk* previous = (start ? array[start - 1] : NULL);

        for(unsigned long i = start; i < length; ++i) {
            pixelchunk* current = array[i];

            if(chunk_is_adjacent(current, subject)) {
                if(previous == NULL) {
                    sort_item(array, current, i, next, length);
                    break;
                }

                else if(eligible_count == ADJACENT_COUNT) {
                    DEBUG("adjacent chunks are larger than known size!\n");
                    setError(ASSUMPTION_WRONG);
                    return;
                }
                eligiblesubjects[eligible_count] = i;
                ++eligible_count;            
            }

            else {

            }

            if(i == length - 1) 
            {            
                dont_skip_corners(array, eligiblesubjects, subject, previous, eligible_count, next, length);
            }
        }
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

void sort_boundary(chunkmap* map) {
    chunkshape* shape = map->shape_list;

    while (shape)
    {
        pixelchunk** array = convert_boundary_list_toarray(shape->boundaries, shape->boundaries_length);
        bubble_sort(array, 0, shape->boundaries_length);

        if(isBadError()) {
            DEBUG("bubble_sort failed with code: %d\n", getLastError());
            return;
        }
        convert_array_to_boundary_list(array, shape->boundaries, shape->boundaries_length);
        //prune_boundary(shape->boundaries);       
        shape = shape->next;
        free(array);
    }
}
