#include <stdlib.h>
#include <nanosvg.h>

#include "nsvgmapping.h"
#include "../error.h"
#include "../../test/tools.h"
#include "../types/image.h"
#include "../mapping.h"

void fill_float_array(float* tobefilled, float* fill, int array_length, int max_length) {
    if(array_length > max_length) {
        DEBUG("arrays length must be less than: %d\n", max_length);
        exit(ARRAY_DIFF_SIZE_ERROR);
    }

    for(int i = 0; i < array_length; ++i) {
        tobefilled[i] = fill[i];
    }
}

void fill_strokedash_array(float* strokedash, float* fill, int array_length) {
    DEBUG("filling strokedash array\n");
    fill_float_array(strokedash, fill, array_length, STROKEDASH_LENGTH);
}

void fill_id(char* id, char* fill, int array_length) {
    if(array_length > ID_LENGTH) {
        DEBUG("new id length must be less than: %d\n", BOUNDS_LENGTH);
        exit(ARRAY_DIFF_SIZE_ERROR);
    }
    int fillindex = 0;

    while(fillindex < array_length) {
        id[fillindex] = fill[fillindex];
        ++fillindex;
    }
}

void fill_bounds(float* bounds, float* fill, int array_length) {
    fill_float_array(bounds, fill, array_length, BOUNDS_LENGTH);
}

void fill_beziercurve(float* beziercurve,
    int array_length,
    float x1, float y1, 
    float x2, float y2, 
    float control_x1, float control_y1, 
    float control_x2, float control_y2) {

    if(beziercurve == NULL) {
        DEBUG("array is null\n");
        exit(NULL_ARGUMENT_ERROR);
    }

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

NSVGpath* create_path(image input, coordinate start, coordinate end) {
    NSVGpath* output = calloc(1, sizeof(NSVGpath));
    float* points = calloc(1, sizeof(float) * BEZIERCURVE_LENGTH);
    output->pts = points;
    float boundingbox[4] = { 0, 0, input.width, input.height };

    fill_beziercurve(output->pts, BEZIERCURVE_LENGTH, start.x, start.y, end.x, end.y, 0, 0, 1, 1); //draw the top side of a box
    fill_bounds(output->bounds, boundingbox, BOUNDS_LENGTH);
    output->npts = 2;
    output->closed = 1;
    output->next = NULL;
    return output;
}

NSVGshape* create_shape(chunkmap* map, char* id, long id_length) {    
    NSVGshape* output = calloc(1, sizeof(NSVGshape));
    DEBUG("creating shape, map_p: %d, id_p: %d, id_length: %d, filling shape id\n", map, id, id_length);
    fill_id(output->id, id, ID_LENGTH);

    NSVGpaint fill = {
        NSVG_PAINT_COLOR,
        NSVG_RGB(0, 0, 0)
    };
    output->fill = fill;

    NSVGpaint stroke = {
        NSVG_PAINT_NONE,
        NSVG_RGB(0, 0, 0)
    };
    output->stroke = stroke;
    output->opacity = 1.0;
    output->strokeWidth = 0.0;
    output->strokeDashOffset = 0.0;

    DEBUG("giving shape strokedash\n");
    float strokedash[1] = {0};
    char strokeDashCount = 1;
    fill_strokedash_array(output->strokeDashArray, strokedash, strokeDashCount); //idk if we need this
    output->strokeDashCount = strokeDashCount;
    output->strokeLineJoin = NSVG_JOIN_MITER;
    output->strokeLineCap = NSVG_CAP_BUTT;
    output->miterLimit = 0;
    output->fillRule = NSVG_FILLRULE_NONZERO;
    output->flags = NSVG_FLAGS_VISIBLE;

    DEBUG("filling shape bounds\n");    
    float newbounds[BOUNDS_LENGTH] = {
        0, 0, 
        map->input.width, 
        map->input.height
    };
    fill_bounds(output->bounds, newbounds, BOUNDS_LENGTH);
    
    output->paths = NULL;
    output->next = NULL;
    return output;
}

NSVGimage* create_nsvgimage(float width, float height) {
    NSVGimage* output = calloc(1, sizeof(NSVGimage));
    output->width = width;
    output->height = height;
    output->shapes = NULL;
    return output;
}
