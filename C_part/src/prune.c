#include <stdlib.h>
#include <stdbool.h>

#include "prune.h"
#include "chunkmap.h"
#include "image.h"

/**
 * 
 * 
 * we need to take all the sorted boundaries and remove the ones after jumping to a non adjacent coordinate
this means torus shapes will become fully filled and the inner shape will partially cover it
**/

bool chunk_is_adjacent(pixelchunk* current, pixelchunk* subject) {
    int abs_x_diff = abs(subject->location.x - current->location.x);
    int abs_y_diff = abs(subject->location.y - current->location.y);
    bool xisclose = abs_x_diff <= 1;
    bool yisclose = abs_y_diff <= 1;
    return xisclose && yisclose;
}

void prune_boundary(pixelchunk_list* boundary) {
    pixelchunk_list* previous = NULL;
    pixelchunk_list* current = boundary;

    while(current != NULL) {
        if(previous != NULL &&
            chunk_is_adjacent(current->chunk_p, previous->chunk_p) == false) {
            previous->next = current->next;
        }

        else {
            previous = current;
        }
        current = current->next;
    }
}
