#pragma once

#include <stdio.h>
#include <math.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"

NSVGimage* dcdfill_for_nsvg(image input, vectorize_options options);
NSVGimage* bobsweep_for_nsvg(image input, vectorize_options options);
void free_nsvg(NSVGimage* input);

