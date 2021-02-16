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

inline vector2 vec_sub(vector2 a, vector2 b)
{
    return (vector2) { a.x - b.x, a.y - b.y };
}

inline vector2 vec_add(vector2 a, vector2 b)
{
    return (vector2) { a.x + b.x, a.y + b.y };
}

inline vector2 vec_negate(vector2 a)
{
    return (vector2) { -a.x, -a.y };
}

inline float vec_dot(vector2 a, vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

inline float vec_mag(vector2 a)
{
    return sqrtf((a.x * a.x) + (a.y * a.y));
}

inline float vec3_mag(vector3 a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline float vec_angle_between(vector2 a, vector2 b)
{
    float mag_product = vec_mag(a) * vec_mag(b);

    if (mag_product == 0.f)
        return M_PI * 2.f;
        
    float trig_coefficient = vec_dot(a, b) / mag_product;

    if (trig_coefficient > 1.f || trig_coefficient < -1.f)
        return M_PI * 2.f;

    return acosf(trig_coefficient);
}

inline vector3 vec_cross(vector3 a, vector3 b)
{
    return (vector3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

inline vector2 vec_cross_trunc(vector3 a, vector3 b)
{
    return (vector2){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z };
}

inline vector2 vec_normalize(vector2 a)
{
    float mag = vec_mag(a);
    return (vector2) { a.x / (mag + !mag), a.y / (mag + !mag) };
}