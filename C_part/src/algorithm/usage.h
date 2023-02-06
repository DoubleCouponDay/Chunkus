#pragma once

#include <stdio.h>
#include <math.h>
#include <nanosvg.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <Handleapi.h>
#include <vadefs.h>
#elif __linux__
#include <pthread.h>
#endif

#include "../image.h"
#include "../chunkmap.h"
#include "./algorithm.h"

typedef struct LayerOperation {
    #ifdef _WIN32
    uintptr_t thread;
    #elif __linux__
    pthread_t* thread;
    #endif
    Layer* layer;
} LayerOperation;

void vectorize(image input, vectorize_options options);
void free_nsvg(NSVGimage* input);

