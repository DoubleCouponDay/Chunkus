#pragma once

#include <stdio.h>
#include <math.h>
#include <nanosvg.h>

#include "image.h"
#include "chunkmap.h"


void free_nsvg(NSVGimage* input);

typedef std::unique_ptr<NSVGimage, decltype(&free_nsvg)> nsvg_ptr;

nsvg_ptr dcdfill_for_nsvg(const image& input, vectorize_options options);
nsvg_ptr bobsweep_for_nsvg(const image& input, vectorize_options options);

