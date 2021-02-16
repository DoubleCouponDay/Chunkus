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

enum {
    ADJACENT_COUNT = 9
};

void sort_item(pixelchunk** array, pixelchunk* current, unsigned long i, unsigned long next, unsigned long length) {
    pixelchunk* inbetween = array[next];
    array[next] = current;
    array[i] = inbetween;
    bubble_sort(array, next, length);
}

void bubble_sort(pixelchunk** array, unsigned long start, unsigned long length) {
    unsigned long eligiblesubjects[ADJACENT_COUNT] = {0};
    pixelchunk* subject = array[start];
    unsigned long next = start + 1;
    unsigned long eligible_count = 0;
    
    pixelchunk* previous = (start ? array[start - 1] : NULL);

    for(unsigned long i = next; i < length; ++i) {
        pixelchunk* current = array[i];
        int abs_x_diff = abs(subject->location.x - current->location.x);
        int abs_y_diff = abs(subject->location.y - current->location.y);
        bool xisclose = abs_x_diff <= 1;
        bool yisclose = abs_y_diff <= 1;

        if((xisclose && yisclose)) {
            if(previous == NULL) {
                sort_item(array, current, i, next, length);
                return;
            }

            else if(eligible_count == ADJACENT_COUNT) {
                DEBUG("adjacent chunks are larger than known size!\n");
                setError(ASSUMPTION_WRONG);
                return;
            }
            eligiblesubjects[eligible_count] = i;
            ++eligible_count;            
        }

        if(i == length - 1) 
        {            
            int x_diff = subject->location.x - previous->location.x;
            int y_diff = subject->location.y - previous->location.y;
            vector2 diff = { x_diff, y_diff };

            float smallest_angle = M_PI * 2.f; // Largest angle physically possible
            pixelchunk* most_eligible = NULL;
            unsigned long most_eligible_index = 0;

            for (unsigned long q = 0; q < eligible_count; ++q) {
                pixelchunk* eligible = array[eligiblesubjects[q]];
                int eligible_x_diff = eligible->location.x - subject->location.x;
                int eligible_y_diff = eligible->location.y - subject->location.y;
                vector2 subject_to_eligible = { eligible_x_diff, eligible_y_diff };

                float angle = vec_angle_between(diff, subject_to_eligible);

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
                DEBUG("No Eligible chunks found adjacent\n");
                return;
            }
        }
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

void convert_array_to_boundary_list(pixelchunk** array, pixelchunk_list* previous_list, unsigned long length) {
    pixelchunk_list* current = previous_list;

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
        free(array);
        shape = shape->next;
    }
}
