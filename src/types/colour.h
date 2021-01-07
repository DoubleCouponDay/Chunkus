#pragma once

#include <math.h>
#include <stdbool.h>
#include "image.h"

pixel convert_colorf_to_pixel(pixelF color);

pixelF convert_pixel_to_colorf(pixel pixel);

bool pixelf_equal(pixelF a, pixelF b);