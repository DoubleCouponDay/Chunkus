#pragma once

#include <stdio.h>
#include "../graphics/map.h"
#include "nanosvg.h"

typedef struct pathshape {
    char* moveto;
    char** lineto;
};

