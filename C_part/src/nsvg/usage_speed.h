#pragma once


#include <nanosvg.h>

#include "image.h"
#include "chunkmap.h"

NSVGimage* vectorize_image_speed(image input, vectorize_options options);

void vectorize_debug_speed(image input, vectorize_options options, char* shapefile, char* borderfile);