#pragma once

#include <stdbool.h>

typedef struct colour
{
    unsigned char r, g, b;
} colour;

typedef struct {
    int x;    
    int y;
    int x_unit_length;
    int y_unit_length;
} coordinate;

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


pixel convert_colorf_to_pixel(pixelF input);

pixelF convert_pixel_to_colorf(pixel input);
colour convert_pixel_to_colour(pixel input);
colour convert_pixelf_to_colour(pixelF input);
pixelF convert_colour_to_pixelf(colour input);

bool pixelf_equal(pixelF a, pixelF b);

inline colour lerp_colours(colour a, colour b, float t)
{
    pixelF a_f = convert_colour_to_pixelf(a);
    pixelF b_f = convert_colour_to_pixelf(b);
    pixelF diff = { a_f.r - b_f.r, a_f.g - b_f.g, a_f.b - b_f.b };

    pixelF output = { a_f.r - diff.r * t, a_f.g - diff.g * t, a_f.b - diff.b * t };
    return convert_pixelf_to_colour(output);
}

int calculate_int_units(int subject);
bool colours_are_similar(pixel color_a, pixel color_b, float max_distance);
char* rgb_to_string(pixel* input);
image create_image(int width, int height);
void free_image_contents(image img);
