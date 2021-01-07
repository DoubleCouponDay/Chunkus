#include "colour.h"
#include <stdio.h>
#include <stdbool.h>

pixel convert_colorf_to_pixel(pixelF color)
{
    pixel out;
    out.r = rintf(color.r * 255.f);
    out.g = rintf(color.g * 255.f);
    out.b = rintf(color.b * 255.f);
    return out;
}

pixelF convert_pixel_to_colorf(pixel pixel)
{
    pixelF out;
    out.r = (float)pixel.r / 255.f;
    out.g = (float)pixel.g / 255.f;
    out.b = (float)pixel.b / 255.f;
    return out;
}

bool pixelf_equal(pixelF a, pixelF b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
