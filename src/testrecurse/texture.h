#pragma once

#include "map.h"
#include "colour.h"

typedef unsigned char uint8;

typedef char char;

typedef struct
{
    RGBcolour Stroke;
    RGBcolour Fill;
} svg_PropertiesF;


// Simple struct with 3 bytes, 1 byte for red, green and blue
typedef union
{
    struct
    {
        uint8 R;
        uint8 G;
        uint8 B;
    };
    uint8 Data[3];
} Pixel;

// Simple struct with 3 floats, 1 for red, green and blue
typedef union
{
    struct
    {
        float R;
        float G;
        float B;
    };
    float Data[3];
} PixelF;

// A container for a dynamically sized array of Pixels
typedef struct
{
    Pixel *pPixels;
    int Width;
    int Height;
} Bitmap;

// A container for dynamically sized array of floating point Pixels
typedef struct
{
    PixelF *pPixels;
    int Width;
    int Height;
} BitmapF;

typedef enum
{
	LINE_LIST = 0,
	LIST_STRIP = 1,
} DRAW_TYPE;
