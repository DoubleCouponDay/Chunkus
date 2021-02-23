#pragma once

#include "defines.h"
#include <math.h>

typedef struct vector2
{
    float x;
    float y;
} vector2;

typedef struct vector3
{
    float x;
    float y;
    float z;
} vector3;

vector2 vec_sub(vector2 a, vector2 b);
vector2 vec_add(vector2 a, vector2 b);
vector2 vec_negate(vector2 a);
float vec_dot(vector2 a, vector2 b);
float vec_mag(vector2 a);
float vec3_mag(vector3 a);
float vec_angle_between(vector2 a, vector2 b);
vector3 vec_cross(vector3 a, vector3 b);
vector2 vec_cross_trunc(vector3 a, vector3 b);
vector2 vec_normalize(vector2 a);