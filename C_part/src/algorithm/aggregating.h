#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "../chunkmap.h"
#include "splitting.h"

#define MAX_BITS 64

bool** OR_64(splits* input, int width, int height);

void free_aggregate(bool** input, int width);
