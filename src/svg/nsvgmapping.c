#include <stdlib.h>

#include "../error.h"
#include "nsvgmapping.h"
#include "../../test/tools.h"

void fill_id(char* id, char* fill, int array_length) {
    if(array_length != ID_LENGTH) {
        DEBUG("arrays must equal length: %d\n", BOUNDS_LENGTH);
        exit(ARRAY_DIFF_SIZE_ERROR);
    }
    int fillindex = 0;

    while(fill[fillindex] != NULL && fillindex < ID_LENGTH) {
        id[fillindex] = fill[fillindex];
    }
}

void fill_bounds(float* bounds, float* fill, int array_length) {
    if(array_length != BOUNDS_LENGTH) {
        DEBUG("arrays must equal length: %d\n", BOUNDS_LENGTH);
        exit(ARRAY_DIFF_SIZE_ERROR);
    }

    for(int i = 0; i < BOUNDS_LENGTH; ++i) {
        bounds[i] = fill[i];
    }
}

void fill_beziercurve(float* beziercurve,
    int array_length,
    float x1, float y1, 
    float x2, float y2, 
    float control_x1, float control_y1, 
    float control_x2, float control_y2) {

    if(array_length != BEZIERCURVE_LENGTH) {
        DEBUG("beziercurve array must be 8 long.");
        exit(ARRAY_DIFF_SIZE_ERROR);
    }
    beziercurve[0] = x1;
    beziercurve[1] = y1;
    beziercurve[2] = x2;
    beziercurve[3] = y2;
    beziercurve[4] = control_x1;
    beziercurve[5] = control_y1;
    beziercurve[6] = control_x2;
    beziercurve[7] = control_y2;
}
