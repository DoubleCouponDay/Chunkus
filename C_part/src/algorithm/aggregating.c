#pragma once

#include "aggregating.h"

#include <stdlib.h>
#include <stdio.h>

#include "../chunkmap.h"
#include "splitting.h"

uint64_t getslice(split* input, int startindex, int endindex, int width, int height) {  
    if(endindex - startindex == 64) {
        uint64_t buffers[64];
        int bitshift = 63;
        int offset = 0;
        
        for(int i = 0 ; i < 64; ++i) {
            uint64t currentbuffer = ((uint64_t)input.nodes[startindex][height].is_boundary) << bitshift);
            buffers[i] = currentbuffer;
            --bitshift;
            ++offset;
        }
    }
}

/// @brief returns a 2D array of ints
/// @param input 
/// @param width 
/// @param height 
/// @return 
uint64_t* OR_64(splits* input, int width, int height) {
    //1. take each row in 64 bit slices
    //2. make 8 variables for each slice
    //3. bitwise OR them together
    //4. write to a single chunkmap
    //5. do it for every slice in the current row
    bool** output = calloc(1, sizeof(bool) * height);

    for(int y = 0; y < height; ++y) {
        int startindex = 0;
        int endindex = 0;
        int widthindex = width - 1;
        output[y] = calloc(1, sizeof(bool) * width);

        while(endindex < width) {   
            endindex = endindex + 64;

            if(endindex >= widthindex) {
                endindex = widthindex;
            }
            uint64_t split1slice = getslice(input.splits[0], startindex, endindex);
            uint64_t split2slice = getslice(input.splits[1], startindex, endindex);
            uint64_t split3slice = getslice(input.splits[2], startindex, endindex);
            uint64_t split4slice = getslice(input.splits[3], startindex, endindex);
            uint64_t split5slice = getslice(input.splits[4], startindex, endindex);
            uint64_t split6slice = getslice(input.splits[5], startindex, endindex);
            uint64_t split7slice = getslice(input.splits[6], startindex, endindex);
            uint64_t split8slice = getslice(input.splits[7], startindex, endindex);
            uint64_t aggregate = split1slice | split2slice | split3slice | split4slice | split5slice | split6slice | split7slice | split8slice;

            startindex = endindex + 1;
            endindex = 
        }
    }
}
