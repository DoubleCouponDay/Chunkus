
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "chunkmap.h"
#include "../test/debug.h"
#include "utility/error.h"

void bubble_sort(pixelchunk** array, unsigned long start, unsigned long length) {
    pixelchunk* subject = array[start];
    unsigned long next = start + 1;
    for(unsigned long i = next; i < length; ++i) {
        pixelchunk* current = array[i];
        bool xisclose = abs(subject->location.x - current->location.x) == 1;
        bool yisclose = abs(subject->location.y - current->location.y) == 1;

        if(xisclose || yisclose) {
            pixelchunk* inbetween = array[next];
            array[next] = current;
            array[i] = inbetween;
            bubble_sort(array, next, length);
            return;
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

void sort_boundary(chunkshape* shape) {
    pixelchunk** array = convert_boundary_list_toarray(shape->boundaries, shape->boundaries_length);
    bubble_sort(array, 0, shape->boundaries_length);
    convert_array_to_boundary_list(array, shape->boundaries, shape->boundaries_length);
    free(array);
}
