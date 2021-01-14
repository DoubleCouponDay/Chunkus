#pragma once

#include "types/colour.h"

typedef pixelF node_variance;

pixelF calculate_pixel_variance(pixel** pixel_array, int num_colors_x, int num_colors_y);
