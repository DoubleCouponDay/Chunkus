#pragma once


#include <nanosvg.h>

#include "image.h"
#include "chunkmap.h"

NSVGimage* find_shapes_speed(chunkmap* map, float threshold);

NSVGimage* vectorize_image_speed(image input, vectorize_options options);