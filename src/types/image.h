#pragma once

#include "map.h"


typedef unsigned char byte;

typedef struct
{
    double r;
    double g;
    double b;
} pixelD;

// RGB floating point color struct
// Values to be stored as normalized values (0-1)
typedef struct
{
    float r;
    float g;
    float b;
} pixelF;

// RGB 8-bit color struct
// Values stored as values between 0-255
typedef struct
{
    byte r;
    byte g;
    byte b;
    coordinate location; //todo
} pixel;

typedef pixel* pixelp;

typedef struct
{
    pixel** pixels;
    int width;
    int height;
    pixel* topleftcorner; //todo
    pixel* toprightcorner; //todo
    pixel* bottomleftcorner; //todo
    pixel* bottomrightcorner; //todo
} image;

image create_image(int width, int height);

void allocate_image(int width, int height, image *img);

void free_image_contents(image *img);