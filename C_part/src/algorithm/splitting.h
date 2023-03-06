#pragma once

#include "../image.h"
#include "chunkmap.h"

#ifndef splitting
#define NUM_SPLITS 8
#endif

typedef struct split_node
{
    pixel color;
    bool is_boundary;
} split_node;

typedef struct split
{
    split_node** nodes;
} split;

typedef struct splits
{
    split splits[NUM_SPLITS];
    int splits_width;
} splits;

void split_chunks(chunkmap* map, splits* splits_out, float threshold);
void free_splits(splits* splits);
