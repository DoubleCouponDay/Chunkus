#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "../chunkmap.h"

bool write_svg_file(NSVGimage* input);

extern const char* OUTPUT_PATH;