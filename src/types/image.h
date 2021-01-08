#pragma once

#include "map.h"


typedef unsigned char byte;

typedef struct
{
    double r;
    double g;
    double b;
    coordinate location;
} pixelD;

// RGB floating point color struct
// Values to be stored as normalized values (0-1)
typedef struct
{
    float r;
    float g;
    float b;
    coordinate location;
} pixelF;

// RGB 8-bit color struct
// Values stored as values between 0-255
typedef struct
{
    byte r;
    byte g;
    byte b;
    coordinate location;
} pixel;

typedef pixel* pixelp;

typedef struct
{
    int width;
    int height;
    pixel** pixels_array_2d;
    pixel* topleftcorner_p;
    pixel* toprightcorner_p; 
    pixel* bottomleftcorner_p; 
    pixel* bottomrightcorner_p; 
} image;

image create_image(int width, int height);

void free_image_contents(image img);