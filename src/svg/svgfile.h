#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "../mapping.h"

bool write_svg_file(NSVGimage* input, chunkmap* map);
