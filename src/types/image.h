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
    pixel** pixels_array_2d;
    int width;
    int height;
    pixel* topleftcorner_p; //todo
    pixel* toprightcorner_p; //todo
    pixel* bottomleftcorner_p; //todo
    pixel* bottomrightcorner_p; //todo
} image;

image create_image(int width, int height);

void allocate_image(int width, int height, image* output_p);

void free_image_contents(image* img_p);