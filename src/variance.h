#pragma once

#include "types/colour.h"

///the average change in colour of the group in three separate components
typedef pixelF group_variance;

pixelF calculate_pixel_variance(pixel** pixel_array, int num_colors_x, int num_colors_y);
