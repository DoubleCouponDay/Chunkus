#pragma once

#include "aggregating.h"

#include <stdlib.h>
#include <stdio.h>

#include "../chunkmap.h"
#include "splitting.h"

uint64_t getslice(split* input, int startindex, int endindex) {
    if(endindex - startindex == 64) {
        return ((uint64_t)input.[]) //TODO: mash the bits together!!!
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
    bool** output = calloc(1, bool * width);

    for(int x = 0; x < width; ++x) {
        int startindex = 0;
        int endindex = 0;
        output[x] = calloc(1, bool * height);

        while(endindex < height) {   
            endindex = endindex + 64;

            if(endindex >= height) {
                endindex = height - 1;
            }
            uint64_t split1slice = getslice(input[0], startindex, endindex);
            uint64_t split2slice = getslice(input[1], startindex, endindex);
            uint64_t split3slice = getslice(input[2], startindex, endindex);
            uint64_t split4slice = getslice(input[3], startindex, endindex);
            uint64_t split5slice = getslice(input[4], startindex, endindex);
            uint64_t split6slice = getslice(input[5], startindex, endindex);
            uint64_t split7slice = getslice(input[6], startindex, endindex);
            uint64_t split8slice = getslice(input[7], startindex, endindex);
            uint64_t aggregate = split1slice | split2slice | split3slice | split4slice | split5slice | split6slice | split7slice | split8slice;

            startindex = endindex + 1;
            endindex = 
        }
    }
}
