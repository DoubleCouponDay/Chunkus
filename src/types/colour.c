#include "colour.h"
#include <stdio.h>
#include <stdbool.h>

pixel convert_colorf_to_pixel(pixelF input)
{
    pixel out;
    out.r = rintf(input.r * 255.f);
    out.g = rintf(input.g * 255.f);
    out.b = rintf(input.b * 255.f);
    out.location = input.location;
    return out;
}

pixelF convert_pixel_to_colorf(pixel input)
{
    pixelF out;
    out.r = (float)input.r / 255.f;
    out.g = (float)input.g / 255.f;
    out.b = (float)input.b / 255.f;
    out.location = input.location;
    return out;
}

bool pixelf_equal(pixelF a, pixelF b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
