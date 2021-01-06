#pragma once

#include "types/colour.h";

typedef pixelF node_variance;

pixelF calculate_pixel_variance(pixel *colors, int num_colors);

float shifted_data_variance(float *data, int data_len);