#include "aggregating.h"

#include <stdlib.h>
#include <stdio.h>

#include "../chunkmap.h"
#include "splitting.h"
#include "../utility/logger.h"
#include "../utility/error.h"

uint64_t getslice(split* input, int startindex, int endindex, int height) {  
    int size = endindex - startindex;

    uint64_t output;
    int bitshift = 63;
    int offset = 0;

    if(size >= bitshift) { //size cannot be larger than 64
        LOG_ERR("ERROR: buffer size is larger than processor architecture");
        setError(ASSUMPTION_WRONG);
        return -1;
    }
    
    for(int i = 0 ; i < size; ++i) { //if size loops 64 times max it will guarantee the correct max amount of loops
        uint64_t currentbuffer = ((uint64_t)input.nodes[startindex + offset][height].is_boundary) << bitshift);
        output |= currentbuffer;
        --bitshift; //bitshift caps out at 0
        ++offset;
    }
    return output;
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
    bool** output = calloc(1, sizeof(bool*) * width);

    for(int y = 0; y < height; ++y) {
        int startindex = 0;
        int endindex = startindex + MAX_BITS;

        output[y] = calloc(1, sizeof(bool) * height);

        while(endindex < width) {

            if(endindex >= MAX_BITS) {
                endindex = width;
            }
            uint64_t split1slice = getslice(input.splits[0], startindex, endindex, y);
            uint64_t split2slice = getslice(input.splits[1], startindex, endindex, y);
            uint64_t split3slice = getslice(input.splits[2], startindex, endindex, y);
            uint64_t split4slice = getslice(input.splits[3], startindex, endindex, y);
            uint64_t split5slice = getslice(input.splits[4], startindex, endindex, y);
            uint64_t split6slice = getslice(input.splits[5], startindex, endindex, y);
            uint64_t split7slice = getslice(input.splits[6], startindex, endindex, y);
            uint64_t split8slice = getslice(input.splits[7], startindex, endindex, y);
            uint64_t aggregate = split1slice | split2slice | split3slice | split4slice | split5slice | split6slice | split7slice | split8slice;

            startindex = endindex + 1;
            endindex = endindex + MAX_BITS;
        }
        *output[y] = aggregate;
    }
}
