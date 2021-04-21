#pragma once

#include "defines.h"
#include <math.h>


template<class T>
inline T lerp(T a, T b, float t)
{
    return a + (b - a) * t;
}

struct vector2
{
    vector2() = default;
    vector2(float x, float y) : x(x), y(y) {}
    vector2(const vector2& other) : x(other.x), y(other.y) {}

    float x;
    float y;

    inline vector2 operator+(const vector2& other) const { return { x + other.x, y + other.y }; }
    inline vector2 operator-(const vector2& other) const { return { x - other.x, y - other.y }; }
    inline vector2 operator-() const { return { -x, -y }; }
    inline vector2 operator*(float fac) const { return { x * fac, y * fac }; }
    inline vector2 operator/(float fac) const { return { x / fac, y / fac }; }

    inline float dot(const vector2& other) const { return x * other.x + y * other.y; }
    inline float mag() const { return sqrtf(x * x + y * y); }
    inline float sqr_mag() const { return x * x + y * y; }

    inline vector2 normalized() const { if (auto m = mag()) return *this / m; return identity(); }

    inline float angle_between(const vector2& other) const 
    {
        float mag_product = mag() * other.mag();

        if (mag_product == 0.f)
            return (float)M_PI * 2.f;
        
        float trig_coefficient = dot(other) / mag_product;

        if (trig_coefficient > 1.f || trig_coefficient < -1.f)
            return (float)M_PI * 2.f;

        return acosf(trig_coefficient);
    }

    inline static vector2 identity() { return { 0.f, 0.f }; }
};

struct vector3
{
    float x;
    float y;
    float z;

    inline vector3 operator+(const vector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
    inline vector3 operator-(const vector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
    inline vector3 operator-() const { return { -x, -y, -z }; }
    inline vector3 operator*(const vector3& other) const { return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x }; }

    inline float dot(const vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    inline float mag() const { return sqrtf(x * x + y * y + z * z); }
    inline float sqr_mag() const { return x * x + y * y + z * z; }
    
};

inline vector2 vec_cross_trunc(vector3 a, vector3 b)
{
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z };
}