
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "chunkmap.h"
#include "../test/debug.h"
#include "utility/error.h"

void

pixelchunk** convert_boundary_list_toarray(pixelchunk_list* list, unsigned long length) {
    pixelchunk** output = calloc(1, sizeof(pixelchunk*) * length);

    for(int i = 0; i < length; ++i) {
        output[i] = list->chunk_p;
    }
    return output;
}

pixelchunk** sort_boundary(chunkshape* shape) {
    pixelchunk** array = convert_boundary_list_toarray(shape->boundaries, shape->boundaries_length);
    quicksort(array, 0, shape->boundaries_length);
    return array;
}
