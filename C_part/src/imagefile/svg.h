#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "chunkmap.h"
#include "nsvg/usage.h"

bool write_svg_file(const nsvg_ptr& input);

extern const char* OUTPUT_PATH;