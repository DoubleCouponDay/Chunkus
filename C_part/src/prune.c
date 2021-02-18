#include <stdlib.h>
#include <stdbool.h>

#include "prune.h"
#include "chunkmap.h"
#include "image.h"

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
            pixelchunk_list* tmp = current;
            current = current->next;
            free(tmp); //memory leak averted
            continue;
        }

        else {
            previous = current;
        }
        current = current->next;
    }
}
