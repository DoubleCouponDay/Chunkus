#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"
#include "../utility/vec.h"

enum mapping_consts {
    BEZIERCURVE_LENGTH = 8,
    BOUNDS_LENGTH = 4,
    MAX_ID_LENGTH = 64,
    STROKEDASH_LENGTH = 8
};

void fill_id(char* dst_id, const char* src_id, int array_length);
void fill_bounds(float* bounds, float* fill, int array_length);
void fill_strokedash_array(float* strokedash, float* fill, int array_length);

void fill_beziercurve(float* beziercurve,
    int array_length,
    float x1, float y1, 
    float x2, float y2, 
    float control_x1, float control_y1, 
    float control_x2, float control_y2);

NSVGpath* create_path(image input, vector2 start, vector2 end);
NSVGshape* create_shape(chunkmap* map, const char* id, long id_length);
NSVGimage* create_nsvgimage(float width, float height);
