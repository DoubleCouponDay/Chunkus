#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "../chunkmap.h"

bool write_svg_file(NSVGimage* input, chunkmap* map);
