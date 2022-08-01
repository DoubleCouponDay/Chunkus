#pragma once

#include <stdio.h>
#include <math.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"

NSVGimage* vectorize(image input, vectorize_options options);
void free_nsvg(NSVGimage* input);

