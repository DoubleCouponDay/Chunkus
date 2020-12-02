#pragma once

#include <math.h>


typedef unsigned char byte;

// RGB 8-bit color struct
// Values stored as values between 0-255
typedef struct
{
    byte r;
    byte g;
    byte b;
} pixel;

typedef pixel* pixelp;

// RGB floating point color struct
// Values to be stored as normalized values (0-1)
typedef struct
{
    float r;
    float g;
    float b;
} colorf;

static inline pixel convert_colorf_to_pixel(colorf color)
{
    pixel out;
    out.r = rintf(color.r * 255.f);
    out.g = rintf(color.g * 255.f);
    out.b = rintf(color.b * 255.f);
    return out;
}

static inline colorf convert_pixel_to_colorf(pixel pixel)
{
    colorf out;
    out.r = (float)pixel.r / 255.f;
    out.g = (float)pixel.g / 255.f;
    out.b = (float)pixel.b / 255.f;
    return out;
}
