#pragma once

#include <math.h>
#include <stdbool.h>

typedef unsigned char byte;

// RGB 8-bit color struct
// Values stored as values between 0-255
typedef struct
{
    byte r;
    byte g;
    byte b;
} pixel;

typedef struct
{
    double r;
    double g;
    double b;
} pixelD;

typedef pixel* pixelp;

// RGB floating point color struct
// Values to be stored as normalized values (0-1)
typedef struct
{
    float r;
    float g;
    float b;
} pixelF;

pixel convert_colorf_to_pixel(pixelF color);

pixelF convert_pixel_to_colorf(pixel pixel);

bool pixelf_equal(pixelF a, pixelF b);