#pragma once

void fill_id(char* id_array, float* fill, int array_length);

void fill_bounds(float* bounds, float* fill, int array_length);

void fill_beziercurve(float* beziercurve,
    int array_length,
    float x1, float y1, 
    float x2, float y2, 
    float control_x1, float control_y1, 
    float control_x2, float control_y2);
